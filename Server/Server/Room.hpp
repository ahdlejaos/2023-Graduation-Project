#pragma once
#include "Session.hpp"

class Room
{
public:
	constexpr Room(unsigned place)
		: myPlace(place)
		, myState(srv::RoomStates::IDLE)
	{

	}

	~Room()
	{}

	const unsigned myPlace;

	atomic<srv::RoomStates> myState;

	std::array<shared_ptr<srv::Session>, srv::MAX_PLAYERS_PER_ROOM> myPlayers;
};
