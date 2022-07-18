#include "pch.hpp"
#include "Framework.hpp"

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider(), concurrentsNumber(0), myWorkers()
	, everyRooms(), everySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, myPipelineBreaker()
{
	std::cout.imbue(std::locale{ "KOREAN" });
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	WSACleanup();
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

		th.join();
	}

	std::cout << "서버 시작\n";
}

void Framework::Update()
{
	while (true)
	{
		SleepEx(10, TRUE);
	}

	std::cout << "서버 종료 중...\n";
}

void Framework::Release()
{
	myPipelineBreaker.request_stop();

	std::cout << "서버 종료\n";
}

void Framework::ProceedAsync(Asynchron* context, ULONG_PTR key, int bytes)
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

void Framework::ProceedConnect(Asynchron* context)
{
	const SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]]
	{
		if (numberUsers < srv::MAX_USERS) [[likely]]
		{
			AcceptPlayer(target);
		}
		else
		{
			std::cout << "유저 수가 초과하여 더 이상 접속을 받을 수 없습니다.\n";
			closesocket(target);
		}
	}
}

void Framework::ProceedSent(Asynchron* context, ULONG_PTR key, int bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

}

void Framework::ProceedRecv(Asynchron* context, ULONG_PTR key, int bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

}

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool)
{
	auto token = stopper.get_token();

	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED* asyncer = nullptr;

	BOOL result{};

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 시작\n";

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(asyncer));

		if (token.stop_requested())
		{
			std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
			break;
		}

		if (TRUE == result) [[likely]]
		{
			me.ProceedAsync(static_cast<Asynchron*>(asyncer), key, static_cast<int>(bytes));
		}
		else
		{
		}
	}

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
}

void Framework::BuildSessions()
{
	for (unsigned i = 0; i < srv::MAX_ENTITIES; i++)
	{
		auto& session = everySessions[i];
		session = make_shared<Session>(i);
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
	const auto place = SeekNewbiePlace();
	auto session = place.get();

	std::cout << "플레이어 접속: " << target << "\n";
	session->Acquire();

	session->SetState(srv::SessionStates::ACCEPTED);
	session->SetSocket(target);
	session->SetID(MakeNewbieID());

	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();
	session->Send(asynchron);

	session->Release();

	return place;
}

shared_ptr<Session> Framework::ConnectPlayer(unsigned place)
{
	auto& session = everySessions.at(place);

	session->Acquire();

	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();

	session->Release();

	return session;
}

void Framework::Dispose(unsigned place)
{

}

void Framework::Dispose(Session * session)
{

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
