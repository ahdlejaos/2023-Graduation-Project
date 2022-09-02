#pragma once
#include "Session.hpp"

namespace srv
{
	class Room : public std::enable_shared_from_this<Room>
	{
	protected:
		constexpr Room(unsigned place)
			: myPlace(place)
			, myState(RoomStates::IDLE)
			, myPlayers(), myEntities()
		{}

	public:
		virtual ~Room()
		{}

		[[nodiscard]] inline static shared_ptr<Room> Create(unsigned place) noexcept
		{
			return shared_ptr<Room>(new Room{ place });
		}

		inline shared_ptr<Room> Handle() noexcept
		{
			return shared_from_this();
		}

		void Update(const float delta_time)
		{

		}

		const unsigned myPlace;
		atomic<RoomStates> myState;

		std::array<shared_ptr<Session>, MAX_PLAYERS_PER_ROOM> myPlayers;
		std::array<shared_ptr<Session>, MAX_NPCS_PER_ROOM> myEntities;
	};
}
