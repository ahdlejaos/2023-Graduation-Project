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

	inline void lock(const std::memory_order order = std::memory_order::memory_order_acquire) volatile noexcept
	{
		while (mySwitch.test_and_set(order));
	}

	inline bool try_lock(const std::memory_order order = std::memory_order::memory_order_acq_rel) volatile noexcept
	{
		return !mySwitch.test_and_set(order);
	}

	inline void unlock(const std::memory_order order = std::memory_order::memory_order_release) volatile noexcept
	{
		mySwitch.clear(order);
	}

	Spinlock(const Spinlock&) = delete;
	Spinlock& operator=(const Spinlock&) = delete;

private:
	atomic_flag mySwitch;
};
