#pragma once
#include "Asynchron.hpp"

class ConnectService
{
public:
	ConnectService()
		: connectWorker(Operation::ACCEPT)
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

	void Awake(SOCKET listener, HANDLE iocp)
	{
		serverSocket = listener;
		serverCompletionPort = iocp;
	}

	void Start()
	{
		auto newbie = connectNewbie.load(std::memory_order_relaxed);

		Accept(newbie);
	}

	void Update()
	{
		const auto new_sock = CreateSocket();
		connectNewbie.store(new_sock);

		Accept(new_sock);
	}

	SOCKET serverSocket;
	HANDLE serverCompletionPort;

	Asynchron connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	const int connectSize;

	atomic<SOCKET> connectNewbie;
	std::condition_variable connectFlag;

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
