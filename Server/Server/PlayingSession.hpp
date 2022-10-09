#pragma once
#include "Session.hpp"

namespace srv
{
	class PlayingSession : public Session
	{
	protected:
		constexpr PlayingSession(unsigned place, db::Service& db_service)
			: Session(place, db_service)
		{}

	public:
		virtual ~PlayingSession()
		{}

		[[nodiscard]] inline static Session* Create(unsigned place, db::Service& db_service) noexcept
		{
			return new PlayingSession{ place, db_service };
		}

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
