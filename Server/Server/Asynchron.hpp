#pragma once
#include "Context.hpp"

namespace srv
{
	class Asynchron : public Context<Asynchron>
	{
	public:
		constexpr Asynchron(const Operations& service)
			: Asynchron(service, {})
		{
			myBuffer.buf = myData;
			myBuffer.len = 0;
		}

		constexpr Asynchron(const Operations& op, const WSABUF& wbuffer)
			: Context(op)
			, myBuffer(wbuffer), myData()
		{
			myBuffer.buf = myData;
			myBuffer.len = wbuffer.len;

			//CopyMemory(wbuffer.buf, myData, wbuffer.len);
			std::copy(wbuffer.buf, wbuffer.buf + wbuffer.len, std::ranges::begin(myData));
		}

		constexpr Asynchron(const Operations& op, WSABUF&& wbuffer)
			: Context(op)
			, myBuffer(std::forward<WSABUF>(wbuffer)), myData()
		{
			myBuffer.buf = myData;
			myBuffer.len = std::forward<ULONG>(wbuffer.len);

			std::ranges::copy_n(std::forward<CHAR*>(wbuffer.buf), std::forward<ULONG>(wbuffer.len), std::ranges::begin(myData));

			//std::copy(wbuffer.buf, wbuffer.buf + std::forward<ULONG>(wbuffer.len), std::ranges::begin(myData));
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

		inline int Send(const SOCKET target, LPDWORD bytes, DWORD flags)
		{
			return WSASend(target, &myBuffer, 1, bytes, flags, this, nullptr);
		}

		inline int Recv(const SOCKET target, LPDWORD bytes, DWORD flags)
		{
			return WSARecv(target, &myBuffer, 1, bytes, &flags, this, nullptr);
		}

		inline int Recv(const SOCKET target)
		{
			return Recv(target, nullptr, 0);
		}

		inline void Release() noexcept
		{
			if (myBuffer.buf)
			{
				myBuffer.buf = nullptr;
				myBuffer.len = 0;
			}
		}

		inline void Clear() noexcept
		{
			ZeroMemory(myData, sizeof(myData));
			myBuffer.len = 0;
		}

		inline constexpr WSABUF& GetBuffer() noexcept
		{
			return myBuffer;
		}

		inline constexpr const WSABUF& GetBuffer() const noexcept
		{
			return myBuffer;
		}

		WSABUF myBuffer;
		CHAR myData[BUFSIZ];
	};

	inline constexpr Asynchron* CreateAsynchron(const Operations& op)
	{
		return new Asynchron(op);
	}
}
