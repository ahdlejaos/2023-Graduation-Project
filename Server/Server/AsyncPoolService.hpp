#pragma once

class AsyncPoolService
{
public:
	constexpr AsyncPoolService()
		: completionPort(NULL)
	{}

	HANDLE completionPort;
};
