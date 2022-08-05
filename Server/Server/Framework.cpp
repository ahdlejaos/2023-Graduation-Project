#include "pch.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "PlayingSession.hpp"
#include "Packet.hpp"

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

Framework::Framework(unsigned int concurrent_hint)
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider()
	, concurrentsNumber(concurrent_hint), concurrentWatcher(concurrent_hint)
	, myWorkers(), workersBreaker()
	, everyRooms(), everySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, login_succeed(), login_failure(), cached_pk_server_info(0, srv::MAX_USERS, srv::GAME_VERSION)
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	workersBreaker.request_stop();

	Release();

	WSACleanup();

	for (auto& th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
}

void Framework::Awake(unsigned short port_tcp)
{
	std::cout << "������ �غ��ϴ� ��...\n";

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "�ڿ��� �ҷ����� ��...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
	std::cout << "�ڿ��� �ҷ����� �Ϸ�\n";
}

void Framework::Start()
{
	std::cout << "������ �����ϴ� ��...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, std::ref(workersBreaker), std::ref(*this), std::ref(myAsyncProvider));
	}

	std::cout << "���� ���۵�!\n";
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			if (concurrentWatcher.try_wait())
			{
				std::cout << "���� ���� ��...\n";

				break;
			}

			SleepEx(10, TRUE);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "���ܷ� ���� ���� ���ͷ�Ʈ: " << e.what() << std::endl;
	}
}

void Framework::Release()
{
	std::cout << "���� ����\n";
}

void Framework::ProceedAsync(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
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

void Framework::ProceedAccept(srv::Asynchron* context)
{
	const SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]]
	{
		if (CanAcceptPlayer()) [[likely]]
		{
			AcceptPlayer(target);
		}
		else
		{
			std::cout << "���� ���� �ʰ��Ͽ� �� �̻� ������ ���� �� �����ϴ�.\n";

			// ��������� ���� ����
			if (FALSE == DisconnectEx(target, nullptr, 0, 0)) [[unlikely]]
			{
				std::cout << "������ ���� �� ���� ���߿� ���� ���ᰡ �����߽��ϴ�.\n";
			}
			else
			{
				myEntryPoint.Push(target);
			}
		}
	}
}

void Framework::ProceedDiconnect(srv::Asynchron* context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "������ ���� ������ ����! (Ű: " << key << ")\n";
	};

	Disconnect(session);
}

void Framework::ProceedSent(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

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

void Framework::ProceedRecv(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

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

			const auto result = session->Recv(BUFSIZ); // �������� ù��° ����
			if (srv::CheckError(result))
			{
				const int error = WSAGetLastError();
				if (!srv::CheckPending(error))
				{
					std::cout << "ù ���ſ��� ���� �߻�! (ID: " << key << ")\n";
				}
			}
		}
	}
	else
	{
		// ��Ŷ ó��
		const auto result = session->Swallow(BUFSIZ, bytes);

		if (result)
		{
			const auto& packet = result.value();
			const auto& pk_type = packet->;

		}
	}
}

void Framework::ProceedDispose(srv::Asynchron* context, ULONG_PTR key)
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

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED* overlap = nullptr;

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

		auto asynchron = static_cast<srv::Asynchron*>(overlap);
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

	me.concurrentWatcher.arrive_and_wait();
}

void Framework::BuildSessions()
{
	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto& user : user_sessions)
	{
		user = static_pointer_cast<Session>(make_shared<PlayingSession>(place++));
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto& npc : npc_sessions)
	{
		npc = make_shared<Session>(place++);
	}
}

void Framework::BuildRooms()
{
	for (unsigned i = 0; i < srv::MAX_ROOMS; i++)
	{
		auto& room = everyRooms[i];
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
	std::cout << "���� �÷��̾� ����: " << target << "\n";

	auto users_number = numberUsers.load(std::memory_order_acquire);

	// ���� �� ����
	cached_pk_server_info.usersCount = users_number;

	// ���� ���� ����
	auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);
	newbie->BeginSend(asynchron);
	newbie->Release();

	numberUsers.store(users_number + 1, std::memory_order_release);
	auto bb = srv::CreatePacket<srv::SCPacketSignInSucceed>(srv::SIGNIN_CAUSE::SUCCEED);

	auto cc = srv::CreateLocalPacket<srv::SCPacketSignInSucceed>(srv::SIGNIN_CAUSE::SUCCEED);

	return newbie;
}

shared_ptr<Session> Framework::ConnectPlayer(unsigned place)
{
	return ConnectPlayer(GetSession(place));
}

shared_ptr<Session> Framework::ConnectPlayer(shared_ptr<Session> session)
{
	session->Acquire();
	std::cout << "�÷��̾� ����: " << session->myID << "\n";

	// �α��� ���� ���� ����
	auto [ticket, asynchron] = srv::CreateTicket(srv::SCPacketSignInSucceed{ srv::SIGNIN_CAUSE::SUCCEED });
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::Disconnect(unsigned place)
{
	Disconnect(GetSession(place).get());
}

void Framework::Disconnect(shared_ptr<Session> session)
{
	Disconnect(session.get());
}

void Framework::Disconnect(Session* session)
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
		, [&](const shared_ptr<Session>& ptr) {
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

template<std::unsigned_integral Integral>
int Framework::SendTo(Session* session, void* const data, const Integral size)
{
	auto asynchron = srv::CreateAsynchron(srv::Operations::SEND);

	auto& wbuffer = asynchron->myBuffer;
	wbuffer.buf = reinterpret_cast<char*>(data);
	wbuffer.len = size;

	return session->BeginSend(asynchron);
}

int Framework::SendServerStatus(Session* session)
{
	auto asynchron = srv::CreateAsynchron(srv::Operations::SEND);

	return 0;
}

int Framework::SendLoginResult(Session* session, login_succeed_t info)
{
	return 0;
}

int Framework::SendLoginResult(Session* session, login_failure_t info)
{
	return 0;
}

bool Framework::CanAcceptPlayer() const noexcept
{
	return numberUsers.load(std::memory_order_acq_rel) < srv::MAX_USERS;
}

bool Framework::CanCreateRoom() const noexcept
{
	return numberRooms.load(std::memory_order_acq_rel) < srv::MAX_ROOMS;
}

shared_ptr<Session> Framework::GetSession(unsigned place) const noexcept(false)
{
	return everySessions.at(place);
}
