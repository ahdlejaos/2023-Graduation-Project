#pragma once

class Asynchron : public WSAOVERLAPPED
{
public:
	constexpr Asynchron(const Operation& service)
		: myService(service)
		, myBuffer()
	{}

	~Asynchron()
	{
		Release();
	}

	constexpr void SetBuffer(const WSABUF& wbuffer) noexcept
	{
		myBuffer = wbuffer;
	}

	constexpr void SetBuffer(WSABUF&& wbuffer) noexcept
	{
		myBuffer = std::forward<WSABUF>(wbuffer);
	}

	constexpr void SetBuffer(char*& buffer, size_t length) noexcept
	{
		myBuffer.buf = buffer;
		myBuffer.len = static_cast<ULONG>(length);
	}

	inline int Send(SOCKET target, LPDWORD bytes, DWORD flags)
	{
		return WSASend(target, &myBuffer, 1, bytes, flags, this, nullptr);
	}

	inline int Recv(SOCKET target, LPDWORD bytes, DWORD flags)
	{
		return WSARecv(target, &myBuffer, 1, bytes, &flags, this, nullptr);
	}

	inline void Release() noexcept
	{
		if (myBuffer.buf)
		{
			delete[myBuffer.len] myBuffer.buf;

			myBuffer.buf = nullptr;
			myBuffer.len = 0;
		}
	}

	inline void Clear() noexcept
	{
		ZeroMemory(this, sizeof(WSAOVERLAPPED));
	}

	const Operation myService;
	WSABUF myBuffer;
};
