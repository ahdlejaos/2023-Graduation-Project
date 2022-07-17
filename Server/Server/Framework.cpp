#include "pch.hpp"
#include "Framework.hpp"

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider(), concurrentsNumber(0), myWorkers()
	, everyRooms(), everySessions()
	, numberRooms(0)
	, lastPacketType(srv::Protocol::NONE)
	, myPipelineBreaker()
{
	setlocale(LC_ALL, "KOREAN");
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
		myWorkers.emplace_back(Worker, std::ref(myPipelineBreaker), std::ref(*this), std::ref(myAsyncProvider));
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
	SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]]
	{
		if (concurrentsNumber < srv::MAX_USERS) [[likely]]
		{
			const auto place = SeekNewbiePlace();
			AcceptPlayer(target, place);
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

unsigned Framework::SeekNewbiePlace() const noexcept
{
	return 0;
}

void Framework::AcceptPlayer(SOCKET target, unsigned place)
{

}

void Framework::ConnectPlayer(unsigned place)
{

}
