#include "pch.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "PlayingSession.hpp"

void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool);

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider(), concurrentsNumber(0), myWorkers()
	, everyRooms(), everySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, myPipelineBreaker()
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	WSACleanup();

	for (auto &th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
}

void Framework::Awake(unsigned int concurrent_hint, unsigned short port_tcp)
{
	std::cout << "���� �õ� ��...\n";

	concurrentsNumber = concurrent_hint;

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "�ڿ� ���� ��...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
}

void Framework::Start()
{
	std::cout << "���� �����ϴ� ��...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto &th = myWorkers.emplace_back(Worker, std::ref(myPipelineBreaker), std::ref(*this), std::ref(myAsyncProvider));
	}

	std::cout << "���� ����\n";
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			SleepEx(10, TRUE);
		}
	}
	catch (std::exception &e)
	{
		std::cout << "���ܷ� ���� ���� ���ͷ�Ʈ: " << e.what() << std::endl;
	}

	std::cout << "���� ���� ��...\n";
}

void Framework::Release()
{
	myPipelineBreaker.request_stop();

	std::cout << "���� ����\n";
}

void Framework::ProceedAsync(Asynchron *context, ULONG_PTR key, unsigned bytes)
{
	const auto operation = context->myOperation;

	switch (operation)
	{
		case srv::Operations::ACCEPT:
		{
			ProceedAccept(context);
		}
		break;

		case srv::Operations::SEND:
		{
			ProceedSent(context, key, bytes);
		}
		break;

		case srv::Operations::RECV:
		{
			ProceedRecv(context, key, bytes);
		}
		break;

		case srv::Operations::DISPOSE:
		{
			ProceedDispose(context, key);
		}
		break;

		default:
		{

		}
		break;
	}
}

void Framework::ProceedAccept(Asynchron *context)
{
	const SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]]
	{
		if (numberUsers < srv::MAX_USERS) [[likely]]
		{
			AcceptPlayer(target);
			numberUsers++;
		}
		else
		{
			std::cout << "���� ���� �ʰ��Ͽ� �� �̻� ������ ���� �� �����ϴ�.\n";

			if (FALSE == DisconnectEx(target, srv::CreateAsynchron(srv::Operations::DISPOSE), 0, 0)) [[unlikely]]
			{
				std::cout << "�񵿱� ���� ������ �����߽��ϴ�.\n";
			}
		}
	}
}

void Framework::ProceedDiconnect(Asynchron *context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "������ ���� ������ ����! (Ű: " << key << ")\n";
	};

	session->Disconnect();
}

void Framework::ProceedSent(Asynchron *context, ULONG_PTR key, unsigned bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "�۽źο��� �߸��� ������ ������! (Ű: " << key << ")\n";
	};

	if (0 < buffer_length)
	{
		const auto left = buffer_length - bytes;
		if (0 < left)
		{
			session->Send(context, static_cast<unsigned>(bytes));
		}
		else if (bytes <= 0)
		{
			std::cout << "�۽� ���� �߻�: ������ ����Ʈ ���� 0��.\n";
			delete context;
		}
		else [[likely]]
		{
			std::cout << "�۽� �Ϸ�.\n";
			delete context;
		}
	}
	else
	{
		delete context;
	}
}

void Framework::ProceedRecv(Asynchron *context, ULONG_PTR key, unsigned bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "���źο��� �߸��� ������ ������! (Ű: " << key << ")\n";
	};

	if (0 == bytes) [[unlikely]] // ���� ������ �̹� GetQueueCompletionStatus���� �Ÿ���
	{
		if (!context->isFirst) [[likely]]
		{
			std::cout << "���� ���� �߻�: ������ ����Ʈ ���� 0��.\n";

			Disconnect(session.get());
		}
		else
		{
			context->isFirst = false; // Page lock ����

			session->Recv(BUFSIZ); // �������� ù��° ����
		}
	}
	else
	{
		// ��Ŷ ó��
		session->Swallow(BUFSIZ, bytes);
	}
}

void Framework::ProceedDispose(Asynchron *context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "������ ���� �߸��� ������ ������! (Ű: " << key << ")\n";
	}
	else
	{
		session->Acquire();
		session->Dispose();
		session->Release();

		numberUsers--;
	}

	delete context;
}

void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED *overlap = nullptr;

	BOOL result{};

	std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested()) [[unlikely]]
		{
			std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";
			break;
		}

		auto asynchron = static_cast<Asynchron *>(overlap);
		if (TRUE == result) [[likely]]
		{
			me.ProceedAsync(asynchron, key, static_cast<int>(bytes));
		}
		else
		{
			me.ProceedDiconnect(asynchron, key);
		}
	}

	std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";
}

void Framework::BuildSessions()
{
	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto &user : user_sessions)
	{
		user = static_pointer_cast<Session>(make_shared<PlayingSession>(place++));
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto &npc : npc_sessions)
	{
		npc = make_shared<Session>(place++);
	}
}

void Framework::BuildRooms()
{
	for (unsigned i = 0; i < srv::MAX_ROOMS; i++)
	{
		auto &room = everyRooms[i];
		room = make_shared<Room>(i);
	}
}

void Framework::BuildResources()
{}

shared_ptr<Session> Framework::AcceptPlayer(SOCKET target)
{
	auto newbie = SeekNewbiePlace();
	newbie->Acquire();
	newbie->Ready(MakeNewbieID(), target);
	std::cout << "�÷��̾� ����: " << target << "\n";
	newbie->Release();

	// ���� ���� ����

	return newbie;
}

shared_ptr<Session> Framework::ConnectPlayer(unsigned place)
{
	return ConnectPlayer(GetSession(place));
}

shared_ptr<Session> Framework::ConnectPlayer(shared_ptr<Session> session)
{
	session->Acquire();

	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::Disconnect(unsigned place)
{
	Disconnect(GetSession(place).get());
}

void Framework::Disconnect(Session *session)
{
	session->Acquire();
	session->Disconnect();
	session->Release();

	std::cout << "���� " << session->myPlace << "�� ���� ����. (���� ��" << numberUsers << "��)\n";
}

shared_ptr<Session> Framework::SeekNewbiePlace() const noexcept
{
	auto players_view = everySessions | std::views::take(srv::MAX_USERS);
	auto it = std::find_if(std::execution::par, players_view.begin(), players_view.end()
		, [&](const shared_ptr<Session> &ptr) {
		return ptr->myState == srv::SessionStates::NONE;
	});

	if (players_view.end() == it)
	{
		return nullptr;
	}
	else
	{
		return (*it);
	}
}

unsigned long long Framework::MakeNewbieID() noexcept
{
	return playerIDs++;
}

shared_ptr<Session> Framework::GetSession(unsigned place) const noexcept(false)
{
	return everySessions.at(place);
}
