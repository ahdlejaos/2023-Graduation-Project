#pragma once

class Asynchron
{
public:
	constexpr Asynchron(const Operation& service)
		: myService(service)
		, myBuffer()
	{}

	~Asynchron()
	{
		if (myBuffer.buf)
		{
			delete myBuffer.buf;
		}
	}

	constexpr void SetBuffer(const WSABUF& wbuffer)
	{
		myBuffer = wbuffer;
	}

	constexpr void SetBuffer(WSABUF&& wbuffer)
	{
		myBuffer = std::forward<WSABUF>(wbuffer);
	}

	const Operation myService;
	WSABUF myBuffer;
};
