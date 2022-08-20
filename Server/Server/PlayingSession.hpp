#pragma once
#include "Session.hpp"

namespace srv
{
	class PlayingSession :public Session
	{
	public:
		constexpr PlayingSession(unsigned place)
			: Session(place)
		{}

		virtual ~PlayingSession()
		{}

		inline constexpr bool IsUser() const noexcept override
		{
			return true;
		}

		inline constexpr bool IsNotUser() const noexcept override
		{
			return false;
		}
	};
}
