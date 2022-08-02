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

		std::cout << "소켓 풀을 구축하는 중... (" << socketsPool.size() << "개)\n";
		for (auto& place : socketsPool)
		{
			place = srv::CreateSocket();
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

		SOCKET newbie = Pop();
		connectNewbie.store(newbie, std::memory_order_relaxed);

		Accept(newbie);
	}

	inline SOCKET Update() noexcept
	{
		// 새로운 접속
		auto newbie = connectNewbie.load(std::memory_order_acquire);

		// 다음 접속을 위한 새로운 TCP 소켓
		connectNewbie.store(Pop(), std::memory_order_release);
		connectWorker.Clear();

		// 다음 접속 받기
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

		if (0 < socketsPool.size())
		{
			const auto last = socketsPool.back();

			socketsPool.pop_back();

			return last;
		}
		else
		{
			return NULL;
		}
	}

	// 소켓을 반환합니다.
	inline void Push(const SOCKET sk) noexcept
	{
		std::scoped_lock locken{ socketPoolLock };

		socketsPool.push_front(sk);
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
			if (!srv::CheckPending(error))
			{
				connectWorker.Clear();

				ZeroMemory(connectBuffer, sizeof(connectBuffer));

				std::cout << "접속 수용 오류!\n";
				//ErrorDisplay("AcceptEx()");
			}
			else
			{
				std::cout << "접속 대기 중: " << target << "\n";
			}
		}
	}

	std::deque<SOCKET> socketsPool;
	Spinlock socketPoolLock;
};
