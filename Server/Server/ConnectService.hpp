#pragma once
#include "Asynchron.hpp"

class ConnectService
{
public:
	ConnectService()
		: serverSocket(NULL), serverEndPoint()
		, connectWorker(srv::Operations::ACCEPT)
		, connectBytes(), connectBuffer()
		, connectNewbie(NULL)
		, connectSize(sizeof(SOCKADDR_IN) + 16)
		, socketsPool(srv::MAX_USERS)
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

		auto end_point_ptr = reinterpret_cast<SOCKADDR *>(&serverEndPoint);
		if (srv::CheckError(bind(serverSocket, end_point_ptr, szAddress))) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::bad_address);
		}

		GUID guidDisconnectEx = WSAID_DISCONNECTEX;
		DWORD cbBytesReturned = 0;
		if (srv::CheckError(WSAIoctl(serverSocket, SIO_GET_EXTENSION_FUNCTION_POINTER
			, &guidDisconnectEx, sizeof(guidDisconnectEx)
			, &DisconnectEx, sizeof(DisconnectEx), &cbBytesReturned
			, NULL, NULL))) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::not_supported);
		}

		for (auto& place : socketsPool)
		{
			//place = srv::CreateSocket();
		}
	}

	inline void Start()
	{
		BOOL option = TRUE;
		if (srv::CheckError(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR
			, reinterpret_cast<char*>(&option), sizeof(option)))) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::operation_not_permitted);
		}

		if (srv::CheckError(listen(serverSocket, srv::MAX_USERS))) [[unlikely]]
		{
			srv::RaiseSystemError(std::errc::network_unreachable);
			return;
		}

		SOCKET newbie = srv::CreateSocket();
		connectNewbie.store(newbie, std::memory_order_relaxed);

		Accept(newbie);
	}

	inline SOCKET Update() noexcept
	{
		// 货肺款 立加
		auto newbie = connectNewbie.load(std::memory_order_seq_cst);

		// 促澜 立加阑 困茄 货肺款 TCP 家南
		connectNewbie.store(srv::CreateSocket(), std::memory_order_release);
		connectWorker.Clear();

		// 促澜 立加 罐扁
		Accept(connectNewbie);

		return newbie;
	}

	inline SOCKET GetLastUser() const noexcept
	{
		return connectNewbie.load(std::memory_order_relaxed);
	}

	inline SOCKET Pop() noexcept
	{
		std::scoped_lock locken{ socketPoolLock };

	}

	inline void Push() noexcept
	{
		std::scoped_lock locken{ socketPoolLock };


	}

	SOCKET serverSocket;
	SOCKADDR_IN serverEndPoint;

	Asynchron connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	const int connectSize;

	atomic<SOCKET> connectNewbie;

private:
	inline void Accept(SOCKET target) noexcept
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

				std::cout << "立加 荐侩 坷幅!\n";
				//ErrorDisplay("AcceptEx()");
			}
			else
			{
				std::cout << "立加: " << target << "\n";
			}
		}
	}

	std::vector<SOCKET> socketsPool;
	std::mutex socketPoolLock;
};
