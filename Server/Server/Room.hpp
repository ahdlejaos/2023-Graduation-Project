#pragma once
#include "Session.hpp"

namespace srv
{
	class Room
	{
	public:
		constexpr Room(unsigned place)
			: myPlace(place)
			, myState(RoomStates::IDLE)
		{

		}

		~Room()
		{}

		const unsigned myPlace;

		atomic<RoomStates> myState;

		std::array<shared_ptr<Session>, MAX_PLAYERS_PER_ROOM> myPlayers;
	};
}
