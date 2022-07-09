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

	void Awake(unsigned int concurrent_hint, unsigned short port_tcp);
	void Start();
	void Update();
	void Release();

	void ProceedAsync(Asynchron* context, ULONG_PTR key, int bytes);
	void ProceedConnect(Asynchron* context);
	void ProceedSent(Asynchron* context, ULONG_PTR key, int bytes);
	void ProceedRecv(Asynchron* context, ULONG_PTR key, int bytes);
	
	friend void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

	void Listen();
	void Accept();
	void Dispose(size_t index);
	void Dispose(Session* session);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;

	unsigned int concurrentsNumber;
	std::vector<std::jthread> myWorkers;
	std::stop_source myPipelineBreaker;

	std::array<shared_ptr<Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, srv::MAX_ENTITIES> everySessions;
	unsigned int numberRooms;

	std::priority_queue<int> timerQueue;

	srv::Protocol lastPacketType;

	std::osyncstream syncout;
};
