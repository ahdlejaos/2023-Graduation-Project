#pragma once
#include "AsyncService.hpp"
#include "ConnectService.hpp"
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
	void ProceedSent(Asynchron* context, int bytes);
	void ProceedRecv(Asynchron* context, int bytes);
	
	void Listen();
	void Accept();
	void Dispose(size_t index);
	void Dispose(Session* session);

private:
	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncService myAsyncProvider;

	std::array<shared_ptr<Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, srv::MAX_ENTITIES> everySessions;
	unsigned int numberRooms;

	std::priority_queue<int> timerQueue;

	srv::Protocol lastPacketType;

	std::osyncstream syncout;
};
