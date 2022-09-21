#pragma once

class AsyncPoolService
{
public:
	constexpr AsyncPoolService()
		: completionPort(NULL)
	{}

	~AsyncPoolService()
	{
		CloseHandle(completionPort);
	}

	inline void Awake(DWORD threads_count = 0) noexcept(false)
	{
		completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threads_count);

		if (NULL == completionPort)
		{
			std::cout << "AsyncPoolService::Awake\n";
			srv::RaiseSystemError(std::errc::not_enough_memory);
		}
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

	inline constexpr HANDLE GetCompletionPort() const noexcept
	{
		return completionPort;
	}

	HANDLE completionPort;
};
