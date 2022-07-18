#pragma once
#include "AsyncPoolService.hpp"
#include "ConnectService.hpp"
#include "Asynchron.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "Packet.hpp"

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

	template<srv::packets PACKET, typename ...Ty>
	std::pair<PACKET*, Asynchron*> CreateTicket(const srv::Protocol protocol, Ty&&... args) const;

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	shared_ptr<Session> AcceptPlayer(SOCKET target);
	shared_ptr<Session> ConnectPlayer(unsigned place);
	void Dispose(unsigned place);
	void Dispose(Session* session);

	shared_ptr<Session> SeekNewbiePlace() const noexcept;

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

template<srv::packets PACKET, typename ...Ty>
inline std::pair<PACKET*, Asynchron*> Framework::CreateTicket(const srv::Protocol protocol, Ty && ...args) const
{
	Asynchron* asyncron = srv::CreateAsynchron(srv::Operations::SEND);

	PACKET* packet = srv::CreatePacket(protocol, std::forward<Ty>(args)...);

	WSABUF wbuffer{};
	wbuffer.buf = reinterpret_cast<char>(packet);
	wbuffer.len = packet->mySize;

	asyncron->SetBuffer(wbuffer);

	return make_pair<PACKET*, Asynchron*>(packet, asyncron);
}
