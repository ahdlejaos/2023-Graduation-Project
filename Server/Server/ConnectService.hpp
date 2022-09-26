#pragma once
#include "Asynchron.hpp"
#include "Spinlock.inl"

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
		, completionPort(NULL)
	{
		WSABUF buffer{};
		buffer.buf = connectBuffer;
		buffer.len = BUFSIZ;

		connectWorker.SetBuffer(std::move(buffer));
	};

	~ConnectService()
	{
		const auto nb_sock = connectNewbie.load(std::memory_order_acquire);
		if (NULL != nb_sock) [[unlikely]] {
			closesocket(nb_sock);
		}

		connectNewbie.store(NULL, std::memory_order_release);

		CloseHandle(completionPort);
	};

	inline void Awake(DWORD threads_count, unsigned short server_port_tcp)
	{
		WSADATA wsadata{};
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata)) [[unlikely]] {
			srv::RaiseSystemError(std::errc::operation_not_supported);
		}

		serverSocket = srv::CreateSocket();
		if (INVALID_SOCKET == serverSocket) [[unlikely]] {
			srv::RaiseSystemError(std::errc::wrong_protocol_type);
			return;
		}

		constexpr int szAddress = sizeof(serverEndPoint);
		ZeroMemory(&serverEndPoint, szAddress);

		serverEndPoint.sin_family = AF_INET;
		serverEndPoint.sin_addr.s_addr = htonl(INADDR_ANY);
		serverEndPoint.sin_port = htons(server_port_tcp);

		auto end_point_ptr = reinterpret_cast<SOCKADDR*>(&serverEndPoint);
		if (srv::CheckError(bind(serverSocket, end_point_ptr, szAddress))) [[unlikely]] {
			srv::RaiseSystemError(std::errc::bad_address);
		}

		GUID guidDisconnectEx = WSAID_DISCONNECTEX;
		DWORD cbBytesReturned = 0;
		if (srv::CheckError(WSAIoctl(serverSocket
			, SIO_GET_EXTENSION_FUNCTION_POINTER
			, &guidDisconnectEx, sizeof(guidDisconnectEx)
			, &DisconnectEx, sizeof(DisconnectEx)
			, &cbBytesReturned
			, NULL, NULL))) [[unlikely]] {
			srv::RaiseSystemError(std::errc::not_supported);
		};

		completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threads_count);

		if (NULL == completionPort)
		{
			srv::RaiseSystemError(std::errc::not_a_socket);
		}

		std::cout << "소켓 풀을 구축하는 중... (" << socketsPool.size() << "개)\n";
		ULONG_PTR id = static_cast<ULONG_PTR>(srv::USERS_ID_BEGIN);
		for (SOCKET& place : socketsPool)
		{
			place = srv::CreateSocket();
			
			Link(place, id);
		}
	}

	inline void Start(const ULONG_PTR server_id)
	{
		BOOL option = TRUE;
		if (srv::CheckError(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR
			, reinterpret_cast<char*>(&option), sizeof(option)))) [[unlikely]] {
			srv::RaiseSystemError(std::errc::operation_not_permitted);
		};
		;
		if (srv::CheckError(listen(serverSocket, srv::MAX_USERS))) [[unlikely]] {
			srv::RaiseSystemError(std::errc::network_unreachable);
			return;
		};

		Link(serverSocket, server_id);

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

	inline void Link(SOCKET target, ULONG_PTR target_id) noexcept(false)
	{
		const auto completion_sock = reinterpret_cast<HANDLE>(target);

		auto handle = CreateIoCompletionPort(completion_sock, completionPort, target_id, 0);

		if (NULL == handle)
		{
			std::cout << "AsyncPoolService::Link\n";
			srv::RaiseSystemError(std::errc::not_a_socket);
		}
	}

	inline BOOL Async(LPDWORD bytes, PULONG_PTR key, LPOVERLAPPED* overlapped, DWORD time = INFINITE)
	{
		return GetQueuedCompletionStatus(completionPort, bytes, key, overlapped, time);
	}

	inline BOOL Post(DWORD info_bytes, ULONG_PTR info_key, LPOVERLAPPED overlapped)
	{
		return PostQueuedCompletionStatus(completionPort, info_bytes, info_key, overlapped);
	}

	// 소켓을 반환합니다.
	inline void Push(const SOCKET sk) noexcept
	{
		std::scoped_lock locken{ socketPoolLock };
		socketsPool.push_front(sk);
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

	inline SOCKET GetLastUser() const noexcept
	{
		return connectNewbie.load(std::memory_order_relaxed);
	}

	inline constexpr HANDLE GetCompletionPort() const noexcept
	{
		return completionPort;
	}

	SOCKET serverSocket;
	SOCKADDR_IN serverEndPoint;

	atomic<SOCKET> connectNewbie;
	srv::Asynchron connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	const int connectSize;

private:
	inline void Accept(SOCKET target) noexcept
	{
		auto result = AcceptEx(serverSocket, target, connectBuffer
			, 0
			, connectSize
			, connectSize
			, &connectBytes, &connectWorker);

		if (FALSE == result) [[unlikely]] {
			auto error = WSAGetLastError();
			if (!srv::CheckPending(error))
			{
				connectWorker.Clear();

				ZeroMemory(connectBuffer, sizeof(connectBuffer));

				std::cout << "접속 수용 오류!\n";
			}
			else
			{
				std::cout << "접속 대기 중: " << target << "\n";
			}
		}
	}

	std::deque<SOCKET> socketsPool;
	Spinlock socketPoolLock;
	HANDLE completionPort;
};
