#pragma once
#include "Session.hpp"

namespace srv
{
	class Room
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

		[[nodiscard]] inline static Room* Create(unsigned place) noexcept
		{
			return new Room{ place };
		}

		void Update(const float delta_time)
		{

		}

		const unsigned myPlace;
		atomic<RoomStates> myState;

		std::array<Session*, MAX_PLAYERS_PER_ROOM> myPlayers;
		std::array<Session*, MAX_NPCS_PER_ROOM> myEntities;
	};
}
