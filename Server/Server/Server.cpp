#include "pch.hpp"
#include "Framework.hpp"

Framework globalFramework{};

int main()
{
	globalFramework.Awake(srv::THREADS_COUNT, srv::SERVER_PORT_TCP);
	globalFramework.Start();
	globalFramework.Update();
	globalFramework.Release();
}
