#pragma once
#include "Asynchron.hpp"

static LPFN_DISCONNECTEX DisconnectEx = nullptr;

class ConnectService
{
public:
	constexpr ConnectService()
		: serverSocket(NULL), serverEndPoint()
		, connectWorker(srv::Operations::ACCEPT)
		, connectBytes(), connectBuffer()
		, connectNewbie(NULL)
		, connectSize(sizeof(SOCKADDR_IN) + 16)
		, everySockets()
	{
		WSABUF buffer{};
		buffer.buf = connectBuffer;
		buffer.len = BUFSIZ;

		connectWorker.SetBuffer(std::move(buffer));
	};

	~ConnectService()
	{
		const auto nb_sock = connectNewbie.load(std::memory_order_acquire);
		if (NULL != nb_sock) [[unlikely]]
		{
			closesocket(nb_sock);
		}
		connectNewbie.store(NULL, std::memory_order_release);
	};

	inline void Awake(unsigned short server_port_tcp)
	{
		WSADATA wsadata{};
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata)) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::operation_not_supported);
		}

		serverSocket = srv::CreateSocket();
		if (INVALID_SOCKET == serverSocket) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::wrong_protocol_type);
			return;
		}

		constexpr int szAddress = sizeof(serverEndPoint);
		ZeroMemory(&serverEndPoint, szAddress);

		serverEndPoint.sin_family = AF_INET;
		serverEndPoint.sin_addr.s_addr = htonl(INADDR_ANY);
		serverEndPoint.sin_port = htons(server_port_tcp);

		if (SOCKET_ERROR == bind(serverSocket, (SOCKADDR*)(&serverEndPoint), szAddress)) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::bad_address);
		}

		GUID guidDisconnectEx = WSAID_DISCONNECTEX;
		DWORD cbBytesReturned = 0;
		WSAIoctl(serverSocket, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guidDisconnectEx, sizeof(guidDisconnectEx)
		, &DisconnectEx, sizeof(DisconnectEx), &cbBytesReturned
		, NULL, NULL);

		for (auto& place : everySockets)
		{
			//place = srv::CreateSocket();
		}
	}

	inline void Start()
	{
		BOOL option = TRUE;
		if (SOCKET_ERROR == setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR
			, reinterpret_cast<char*>(&option), sizeof(option))) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::operation_not_permitted);
		}

		if (SOCKET_ERROR == listen(serverSocket, srv::MAX_USERS)) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::network_unreachable);
			return;
		}

		SOCKET newbie = srv::CreateSocket();
		connectNewbie.store(newbie, std::memory_order_relaxed);

		Accept(newbie);
	}

	inline SOCKET Update()
	{
		// ���ο� ����
		auto newbie = connectNewbie.load(std::memory_order_seq_cst);

		// ���� ������ ���� ���ο� TCP ����
		connectNewbie.store(srv::CreateSocket(), std::memory_order_release);
		connectWorker.Clear();

		// ���� ���� �ޱ�
		Accept(connectNewbie);

		return newbie;
	}

	inline SOCKET GetLastUser() const noexcept
	{
		return connectNewbie.load(std::memory_order_relaxed);
	}

	SOCKET serverSocket;
	SOCKADDR_IN serverEndPoint;

	Asynchron connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	const int connectSize;

	atomic<SOCKET> connectNewbie;

private:
	inline void Accept(SOCKET target) noexcept(false)
	{
		auto result = AcceptEx(serverSocket, target, connectBuffer
			, 0
			, connectSize
			, connectSize
			, &connectBytes, &connectWorker);

		if (FALSE == result) [[unlikely]]
		{
			auto error = WSAGetLastError();
			if (ERROR_IO_PENDING != error)
			{
				connectWorker.Clear();

				ZeroMemory(connectBuffer, sizeof(connectBuffer));

				std::cout << "���� ���� ����!\n";
				//ErrorDisplay("AcceptEx()");
			}
		}
	}

	std::array<SOCKET, srv::MAX_USERS> everySockets;
};
