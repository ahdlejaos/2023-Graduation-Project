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

	template<typename MY_PACKET, typename ...Ty>
		requires std::is_base_of_v<Packet, MY_PACKET>
	std::pair<LPWSABUF, Asynchron*> CreateTicket(Ty&&... args) const;

	void Dispose(size_t index);
	void Dispose(Session* session);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	unsigned SeekNewbiePlace() const noexcept;
	void AcceptNewbie(SOCKET target, unsigned place);

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
};
