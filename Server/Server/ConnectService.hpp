#pragma once
#include "Asynchron.hpp"

class ConnectService
{
public:
	constexpr ConnectService()
		: serverSocket(NULL), serverEndPoint()
		, connectWorker(Operation::ACCEPT)
		, connectBytes(), connectBuffer()
		, connectNewbie(NULL)
		, connectSize(sizeof(SOCKADDR_IN) + 16)
	{
		WSABUF buffer{};
		buffer.buf = connectBuffer;
		buffer.len = BUFSIZ;

		connectWorker.SetBuffer(std::move(buffer));
	};

	~ConnectService()
	{
		const auto nb_sock = connectNewbie.load(std::memory_order_acquire);
		if (NULL != nb_sock)
		{
			closesocket(nb_sock);
		}
		connectNewbie.store(NULL, std::memory_order_release);
	};

	inline void Awake(unsigned short server_port)
	{
		WSADATA wsadata{};
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		{
			srv::RaiseSystemError(std::errc::operation_not_supported);
		}

		serverSocket = srv::CreateSocket();
		if (INVALID_SOCKET == serverSocket)
		{
			srv::RaiseSystemError(std::errc::wrong_protocol_type);
			return;
		}

		constexpr int szAddress = sizeof(serverEndPoint);
		ZeroMemory(&serverEndPoint, szAddress);

		serverEndPoint.sin_family = AF_INET;
		serverEndPoint.sin_addr.s_addr = htonl(INADDR_ANY);
		serverEndPoint.sin_port = htons(server_port);

		if (SOCKET_ERROR == bind(serverSocket, (SOCKADDR*)(&serverEndPoint), szAddress))
		{
			srv::RaiseSystemError(std::errc::bad_address);
		}
	}

	inline void Start()
	{

		auto newbie = connectNewbie.load(std::memory_order_relaxed);

		Accept(newbie);
	}

	inline void Update()
	{
		const auto new_sock = srv::CreateSocket();
		connectNewbie.store(new_sock);

		Accept(new_sock);
	}

	SOCKET serverSocket;
	SOCKADDR_IN serverEndPoint;

	Asynchron connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	const int connectSize;

	atomic<SOCKET> connectNewbie;

private:
	void Accept(SOCKET target)
	{
		auto result = AcceptEx(serverSocket, target, connectBuffer
			, 0
			, connectSize
			, connectSize
			, &connectBytes, &connectWorker);

		if (FALSE == result)
		{
			auto error = WSAGetLastError();
			if (ERROR_IO_PENDING != error)
			{
				connectWorker.Clear();

				ZeroMemory(connectBuffer, sizeof(connectBuffer));

				//ErrorDisplay("AcceptEx()");
			}
		}
	}
};
