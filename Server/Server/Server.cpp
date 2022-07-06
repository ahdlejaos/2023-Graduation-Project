#include "pch.hpp"
#include "Framework.hpp"

Framework globalFramework{};

int main()
{
	globalFramework.Awake();
	globalFramework.Start();
	globalFramework.Update();
	globalFramework.Release();
}
