#include "pch.hpp"
#include "Framework.hpp"

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider(), myWorkers()
	, everyRooms(), everySessions()
	, numberRooms(0)
	, lastPacketType(srv::Protocol::NONE)
	, myPipelineBreaker()
	, syncout(std::cout)
{
	setlocale(LC_ALL, "KOREAN");
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	WSACleanup();
}

void Framework::Awake()
{
	std::cout << "서버 시동 중...\n";

	myAsyncProvider.Awake(6);
	myEntryPoint.Awake(6000);
}

void Framework::Start()
{
	std::cout << "서버 시작하는 중...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();



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

void Framework::ProceedAsync(Asynchron* context, int bytes)
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
			ProceedSent(context, bytes);
		}
		break;

		case srv::Operations::RECV:
		{
			ProceedRecv(context, bytes);
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

}

void Framework::ProceedSent(Asynchron* context, int bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

}

void Framework::ProceedRecv(Asynchron* context, int bytes)
{

}

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool)
{
	auto token = stopper.get_token();

	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED* asyncer = nullptr;

	BOOL result;
	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(asyncer));

		if (token.stop_requested())
		{
			std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
			break;
		}

		if (TRUE == result)
		{

		}
		else
		{

		}
	}
}
