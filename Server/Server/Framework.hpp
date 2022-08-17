#pragma once
#include "AsyncPoolService.hpp"
#include "ConnectService.hpp"
#include "Packet.hpp"

class Framework
{
private:
#pragma region Screws
	struct login_succeed_t { srv::SIGNIN_CAUSE cause; };
	struct login_failure_t { srv::SIGNIN_CAUSE cause; };

	login_succeed_t login_succeed;
	login_failure_t login_failure;

	srv::SCPacketServerInfo cached_pk_server_info;
#pragma endregion

public:
	Framework(unsigned int concurrent_hint);
	~Framework();

	void Awake(unsigned short port_tcp);
	void Start();
	void Update();
	void Release();

	void ProceedAsync(srv::Asynchron* context, ULONG_PTR key, unsigned bytes);
	void ProceedAccept(srv::Asynchron* context);
	void ProceedSent(srv::Asynchron* context, ULONG_PTR key, unsigned bytes);
	void ProceedRecv(srv::Asynchron* context, ULONG_PTR key, unsigned bytes);
	void ProceedDispose(srv::Asynchron* context, ULONG_PTR key);
	void ProceedBeginDiconnect(srv::Asynchron* context, ULONG_PTR key);

	friend void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);

	bool CanAcceptPlayer() const noexcept;
	bool CanCreateRoom() const noexcept;

	shared_ptr<Session> GetSession(unsigned place) const noexcept(false);
	shared_ptr<Session> FindSession(unsigned long long id) const noexcept(false);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	shared_ptr<Session> AcceptPlayer(SOCKET target);
	shared_ptr<Session> ConnectPlayer(unsigned place);
	shared_ptr<Session> ConnectPlayer(shared_ptr<Session> session);
	void BeginDisconnect(unsigned place);
	void BeginDisconnect(shared_ptr<Session> session);
	void BeginDisconnect(Session* session);

	shared_ptr<Session> SeekNewbiePlace() const noexcept;
	unsigned long long MakeNewbieID() noexcept;

	template<std::unsigned_integral Integral>
	int SendTo(Session* session, void* const data, const Integral size);
	int SendServerStatus(Session* session);
	int SendLoginResult(Session* session, login_succeed_t info);
	int SendLoginResult(Session* session, login_failure_t info);

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;

	unsigned int concurrentsNumber;
	std::latch concurrentWatcher;
	std::vector<Thread> myWorkers;
	std::stop_source workersBreaker;

	unique_ptr<Thread> timerWorker;
	std::priority_queue<int> timerQueue;

	unique_ptr<Thread> databaseWorker;
	std::priority_queue<int> databaseQueue;

	std::array<shared_ptr<Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, srv::MAX_ENTITIES> everySessions;
	atomic<unsigned> numberRooms;
	atomic<unsigned> numberUsers;

	atomic<unsigned long long> playerIDs;
	srv::Protocol lastPacketType;
};

namespace srv
{
	template<packets Pk>
	let Pair<Pk*, Asynchron*> CreateTicket(const Pk& datagram)
	{
		Asynchron* asyncron=CreateAsynchron(Operations::SEND);

		auto packet=srv::CreatePacket(datagram);

		WSABUF wbuffer{};
		wbuffer.buf=reinterpret_cast<char*>(packet);
		wbuffer.len=packet->mySize;

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}

	template<packets Pk>
	let Pair<Pk*, Asynchron*> CreateTicket(Pk&& datagram)
	{
		Asynchron* asyncron=CreateAsynchron(Operations::SEND);

		auto packet=srv::CreatePacket(std::forward<Pk>(datagram));

		WSABUF wbuffer{};
		wbuffer.buf=reinterpret_cast<char*>(packet);
		wbuffer.len=packet->mySize;

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}

	template<packets Pk, typename ...Ty>
	let Pair<Pk*, Asynchron*> CreateTicket(Ty&& ...args)
	{
		Asynchron* asyncron=CreateAsynchron(Operations::SEND);

		auto packet=srv::CreatePacket<Pk>(std::forward<decltype((args))>(args)...);

		WSABUF wbuffer{};
		wbuffer.buf=reinterpret_cast<char*>(packet);
		wbuffer.len=packet->mySize;

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}
}
