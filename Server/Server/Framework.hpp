#pragma once
#include "AsyncContext.hpp"
#include "Asynchron.hpp"
#include "Room.hpp"
#include "Session.hpp"

class Framework
{
public:
	Framework();
	~Framework();

	void Awake();
	void Start();
	void Update();
	void Release();

	void ProceedAsync(Asynchron* context, int bytes);
	void ProceedConnect(Asynchron* context);
	void ProceedSent(Asynchron* context);
	void ProceedRecv(Asynchron* context);
	
	void Listen();
	void Accept();
	void Dispose(size_t index);
	void Dispose(Session* session);

private:
	SOCKET mySocket;
	ULONG_PTR myID;

	AsyncContext myContext;

	std::array<shared_ptr<Room>, MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, MAX_ENTITIES> everySessions;

	unsigned int numberRooms;

	std::priority_queue<int> timerQueue;

	Protocol lastPacketType = Protocol::NONE;

	std::osyncstream syncout;
};
