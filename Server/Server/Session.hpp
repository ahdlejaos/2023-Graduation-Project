#pragma once
#include "Room.hpp"

class Session
{
public:
	constexpr Session(unsigned place)
		: myPlace(place)
	{

	}

	virtual ~Session()
	{}

	inline void SetState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_relaxed);
	}

	inline void SetSocket(const SOCKET sock)
	{
		mySocket.store(sock, std::memory_order_relaxed);
	}

	inline void SetRoom(const shared_ptr<Room>& room)
	{
		myRoom.store(room, std::memory_order_relaxed);
	}

	inline void SetRoom(shared_ptr<Room>&& room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_relaxed);
	}

	const unsigned int myPlace;

	atomic<srv::SessionStates> myState;
	atomic<SOCKET> mySocket;
	atomic<shared_ptr<Room>> myRoom;
};
