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

	void GetSession(unsigned place) const noexcept(false);

	void ProceedAsync(Asynchron* context, ULONG_PTR key, int bytes);
	void ProceedConnect(Asynchron* context);
	void ProceedSent(Asynchron* context, ULONG_PTR key, int bytes);
	void ProceedRecv(Asynchron* context, ULONG_PTR key, int bytes);
	void ProceedDispose(Asynchron* context, ULONG_PTR key, int bytes);

	friend void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	shared_ptr<Session> AcceptPlayer(SOCKET target);
	shared_ptr<Session> ConnectPlayer(unsigned place);
	void Dispose(unsigned place);
	void Dispose(Session* session);

	shared_ptr<Session> SeekNewbiePlace() const noexcept;
	unsigned long long MakeNewbieID() noexcept;

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;

	unsigned int concurrentsNumber;
	std::vector<std::thread> myWorkers;
	std::stop_source myPipelineBreaker;
	std::priority_queue<int> timerQueue;

	std::array<shared_ptr<Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, srv::MAX_ENTITIES> everySessions;
	atomic<unsigned> numberRooms;
	atomic<unsigned> numberUsers;

	atomic<unsigned long long> playerIDs;
	srv::Protocol lastPacketType;
};

namespace srv
{
	template<packets PACKET, typename ...Ty>
	inline std::pair<PACKET*, Asynchron*> CreateTicket(Ty ...args)
	{
		Asynchron* asyncron = CreateAsynchron(Operations::SEND);

		PACKET* packet = srv::CreatePacket(protocol, std::forward<Ty>(args)...);

		WSABUF wbuffer{};
		wbuffer.buf = reinterpret_cast<char*>(packet);
		wbuffer.len = packet->mySize;

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}
}
