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
	std::cout << "���� �õ� ��...\n";

	myAsyncProvider.Awake(6);
	myEntryPoint.Awake(6000);
}

void Framework::Start()
{
	std::cout << "���� �����ϴ� ��...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();



	std::cout << "���� ����\n";
}

void Framework::Update()
{
	while (true)
	{
		SleepEx(10, TRUE);
	}

	std::cout << "���� ���� ��...\n";
}

void Framework::Release()
{
	myPipelineBreaker.request_stop();

	std::cout << "���� ����\n";
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
			std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";
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
