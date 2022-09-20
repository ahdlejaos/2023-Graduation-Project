#pragma once
#include "Session.hpp"

namespace srv
{
	class PlayingSession : public Session
	{
	protected:
		constexpr PlayingSession(unsigned place, DatabaseService& db_service)
			: Session(place, db_service)
		{}

	public:
		virtual ~PlayingSession()
		{}

		[[nodiscard]] inline static shared_ptr<Session> Create(unsigned place, DatabaseService& db_service) noexcept
		{
			return static_pointer_cast<srv::Session>(shared_ptr<PlayingSession>(new PlayingSession{ place, db_service }));
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
