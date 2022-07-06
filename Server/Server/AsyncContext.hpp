#pragma once

class AsyncContext
{
public:
	constexpr AsyncContext()
		: completionPort(NULL)
	{}

	~AsyncContext()
	{
		CloseHandle(completionPort);
	}

	inline void Awake(DWORD threads_count = 0) noexcept(false)
	{
		completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threads_count);

		if (NULL == completionPort)
		{
			std::cout << "AsyncContext::Awake\n";
			throw std::system_error(std::make_error_code(std::errc::not_enough_memory));
		}
	}

	inline void Link(SOCKET target, ULONG_PTR target_id) noexcept(false)
	{
		const auto completion_sock = reinterpret_cast<HANDLE>(target);

		auto handle = CreateIoCompletionPort(completion_sock, completionPort, target_id, 0);

		if (NULL == handle)
		{
			std::cout << "AsyncContext::Link\n";
			throw std::system_error(std::make_error_code(std::errc::not_a_socket));
		}
	}

	inline constexpr HANDLE GetCompletionPort() const noexcept
	{
		return completionPort;
	}

	HANDLE completionPort;
};
