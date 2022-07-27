#pragma once
#include "Session.hpp"

class PlayingSession :public Session
{
public:
	constexpr PlayingSession(unsigned place)
		: Session(place)
	{}

	virtual ~PlayingSession()
	{}

	inline constexpr bool IsUser() noexcept override
	{
		return true;
	}

	inline constexpr bool IsNotUser() noexcept override
	{
		return false;
	}
};
