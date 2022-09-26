#pragma once

namespace srv
{
	class Asynchron : public WSAOVERLAPPED
	{
	public:
		constexpr Asynchron(const Operations& service)
			: Asynchron(service, {})
		{}

		constexpr Asynchron(const Operations& service, const WSABUF& wbuffer)
			: myOperation(service)
			, myBuffer(wbuffer), myData()
		{
			myBuffer.buf = myData;
			myBuffer.len = wbuffer.len;

			//CopyMemory(wbuffer.buf, myData, wbuffer.len);
			std::copy(wbuffer.buf, wbuffer.buf + wbuffer.len, std::ranges::begin(myData));
		}

		constexpr Asynchron(const Operations& service, WSABUF&& wbuffer)
			: myOperation(service)
			, myBuffer(std::forward<WSABUF>(wbuffer)), myData()
		{
			myBuffer.buf = myData;
			myBuffer.len = std::forward<ULONG>(wbuffer.len);

			std::copy(wbuffer.buf, wbuffer.buf + wbuffer.len, std::ranges::begin(myData));
		}

		~Asynchron()
		{
			Release();
		}

		inline constexpr void SetBuffer(const WSABUF& wbuffer) noexcept
		{
			myBuffer = wbuffer;
		}

		inline constexpr void SetBuffer(WSABUF&& wbuffer) noexcept
		{
			myBuffer = std::forward<WSABUF>(wbuffer);
		}

		inline constexpr void SetBuffer(char* buffer, size_t length) noexcept
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

		inline constexpr WSABUF& GetBuffer() noexcept
		{
			return myBuffer;
		}

		inline constexpr const WSABUF& GetBuffer() const noexcept
		{
			return myBuffer;
		}

		const Operations myOperation;
		WSABUF myBuffer;
		CHAR myData[BUFSIZ];
	};

	inline constexpr Asynchron* CreateAsynchron(const Operations& op)
	{
		return new Asynchron(op);
	}
}
