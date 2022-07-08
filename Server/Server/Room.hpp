#pragma once
#include "Session.hpp"

class Room
{
public:
	constexpr Room(unsigned place)
		: myPlace(place)
	{

	}

	~Room()
	{}

	const unsigned myPlace;

	std::array<shared_ptr<Session>, srv::MAX_PLAYERS_PER_ROOM> myPlayers;
};
