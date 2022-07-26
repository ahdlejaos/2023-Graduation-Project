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

	for (auto& th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
}

void Framework::Awake(unsigned int concurrent_hint, unsigned short port_tcp)
{
	std::cout << "서버 시동 중...\n";

	concurrentsNumber = concurrent_hint;

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "자원 생성 중...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
}

void Framework::Start()
{
	std::cout << "서버 시작하는 중...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, std::ref(myPipelineBreaker), std::ref(*this), std::ref(myAsyncProvider));
	}

	std::cout << "서버 시작\n";
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
	catch (std::exception& e)
	{
		std::cout << "예외로 인한 서버 인터럽트: " << e.what() << std::endl;
	}

	std::cout << "서버 종료 중...\n";
}

void Framework::Release()
{
	myPipelineBreaker.request_stop();

	std::cout << "서버 종료\n";
}

void Framework::ProceedAsync(Asynchron *context, ULONG_PTR key, int bytes)
{
	const auto operation = context->myOperation;

	switch (operation)
	{
		case srv::Operations::ACCEPT:
		{
			ProceedConnect(context);
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

		default:
		{

		}
		break;
	}
}

void Framework::ProceedConnect(Asynchron *context)
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
			std::cout << "유저 수가 초과하여 더 이상 접속을 받을 수 없습니다.\n";

			if (FALSE == DisconnectEx(target, context, 0, 0)) [[unlikely]]
			{
				std::cout << "비동기 연결 해제가 실패했습니다.\n";
			}
		}
	}
}

void Framework::ProceedSent(Asynchron *context, ULONG_PTR key, int bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

}

void Framework::ProceedRecv(Asynchron *context, ULONG_PTR key, int bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "수신부에서 잘못된 세션을 참조함!\n";
	};

	if (0 == bytes) [[unlikely]] // 연결 끊김은 이미 GetQueueCompletionStatus에서 거른다
	{
		if (!context->isFirst) [[likely]]
		{
			Dispose(session.get());
		}
		else
		{
			context->isFirst = false; // Page lock 해제

			session->Recv(BUFSIZ);
		}
	}
	else
	{

	}
}

void Framework::ProceedDispose(Asynchron *context, ULONG_PTR key)
{
	numberUsers--;
}

void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED *overlap = nullptr;

	BOOL result{};

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 시작\n";

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested())
		{
			std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
			break;
		}

		auto asynchron = static_cast<Asynchron *>(overlap);
		if (TRUE == result) [[likely]]
		{
			me.ProceedAsync(asynchron, key, static_cast<int>(bytes));
		}
		else
		{
			me.ProceedDispose(asynchron, key);
		}
	}

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
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
	for (auto& npc : npc_sessions)
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
	const auto place = SeekNewbiePlace();
	auto session = place.get();

	std::cout << "플레이어 접속: " << target << "\n";
	session->Acquire();

	session->SetState(srv::SessionStates::ACCEPTED);
	session->SetSocket(target);
	session->SetID(MakeNewbieID());

	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();
	session->BeginSend(asynchron);

	session->Release();

	return place;
}

shared_ptr<Session> Framework::ConnectPlayer(unsigned place)
{
	auto &session = everySessions.at(place);

	session->Acquire();

	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();

	session->Release();

	return session;
}

void Framework::Dispose(unsigned place)
{
	Dispose(GetSession(place).get());
}

void Framework::Dispose(Session *session)
{
	DisconnectEx(session->mySocket, nullptr, 0, 0);
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
