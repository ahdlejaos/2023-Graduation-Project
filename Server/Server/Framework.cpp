#include "pch.hpp"
#include "Framework.hpp"

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider()
	, everyRooms(), everySessions()
	, numberRooms(0)
	, lastPacketType(srv::Protocol::NONE)
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

	myAsyncProvider.Link(myEntryPoint.serverSocket, srv::SERVER_ID);
	myEntryPoint.Start();

	std::cout << "서버 시작\n";
}

void Framework::Update()
{
	while (true)
	{

	}

	std::cout << "서버 종료 중...\n";
}

void Framework::Release()
{


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

}

void Framework::ProceedRecv(Asynchron* context, int bytes)
{

}
