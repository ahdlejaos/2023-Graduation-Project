#pragma once

class Session
{
public:
	constexpr Session(unsigned place)
		: myPlace(place)
	{

	}

	virtual ~Session()
	{}
	
	const unsigned int myPlace;
	atomic<srv::SessionStates> myState;
	atomic<SOCKET> mySocket;

};
