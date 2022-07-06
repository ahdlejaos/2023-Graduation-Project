#pragma once
#include "Session.hpp"

class Room
{
public:
	std::array<shared_ptr<Session>, srv::MAX_PLAYERS_PER_ROOM> myPlayers;
};
