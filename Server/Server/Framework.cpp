#include "pch.hpp"
#include "Framework.hpp"

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider()
	, everyRooms(), everySessions()
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
	syncout << "서버 시동 중...\n";

	myEntryPoint.Awake(6000);
}

void Framework::Start()
{
	syncout << "서버 시작하는 중...\n";

	myEntryPoint.Start();

	syncout << "서버 시작\n";
}

void Framework::Update()
{

	while (true)
	{

	}
}

void Framework::Release()
{

}

void Framework::ProceedAsync(Asynchron* context, int bytes)
{

}

void Framework::ProceedConnect(Asynchron* context)
{

}

void Framework::ProceedSent(Asynchron* context)
{

}

void Framework::ProceedRecv(Asynchron* context)
{

}
