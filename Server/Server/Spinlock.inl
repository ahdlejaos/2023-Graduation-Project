#pragma once
#include "pch.hpp"

class Spinlock
{
public:
	constexpr Spinlock()
		: mySwitch()
	{}

	~Spinlock()
	{
		mySwitch.clear();
	}

	inline void Lock(const std::memory_order order = std::memory_order::memory_order_acquire) volatile noexcept
	{
		while (mySwitch.test_and_set(order));
	}

	inline bool TryLock(const std::memory_order order = std::memory_order::memory_order_acquire) volatile noexcept
	{
		return !mySwitch.test_and_set(std::memory_order_acquire);
	}

	inline void Unlock(const std::memory_order order = std::memory_order::memory_order_release) volatile noexcept
	{
		mySwitch.clear(order);
	}

	Spinlock(const Spinlock&) = delete;
	Spinlock& operator=(const Spinlock&) = delete;

private:
	atomic_flag mySwitch;
};
