#pragma once
#include "AsyncPoolService.hpp"
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
	
	friend void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

	void Listen();
	void Accept();
	void Dispose(size_t index);
	void Dispose(Session* session);

private:
	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;
	std::vector<std::jthread> myWorkers;

	std::array<shared_ptr<Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, srv::MAX_ENTITIES> everySessions;
	unsigned int numberRooms;

	std::priority_queue<int> timerQueue;

	srv::Protocol lastPacketType;

	std::stop_source myPipelineBreaker;
	std::osyncstream syncout;
};
