#include "pch.hpp"
#include "Framework.hpp"

Framework::Framework()
	: myContext()
	, everyRooms(), everySessions()
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
	WSADATA wsadata{};
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		srv::RaiseSystemError(std::errc::operation_not_supported);
	}

	mySocket = srv::CreateSocket();
	if (INVALID_SOCKET == mySocket)
	{
		srv::RaiseSystemError(std::errc::wrong_protocol_type);
		return;
	}

}

void Framework::Start()
{

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
