#pragma once
#include "AsyncPoolService.hpp"
#include "ConnectService.hpp"
#include "Packet.hpp"
#include "Asynchron.hpp"

class Framework
{
private:
#pragma region Screws
	struct login_succeed_t
	{
		srv::SIGNIN_CAUSE cause;
	};
	struct login_failure_t
	{
		srv::SIGNIN_CAUSE cause;
	};

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

	shared_ptr<srv::Session> GetSession(unsigned place) const noexcept(false);
	shared_ptr<srv::Session> FindSession(unsigned long long id) const noexcept(false);

	using DBinUsersSearcher = bool(const Sentence user_id);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	shared_ptr<srv::Session> AcceptPlayer(SOCKET target);
	shared_ptr<srv::Session> ConnectPlayer(unsigned place);
	shared_ptr<srv::Session> ConnectPlayer(shared_ptr<srv::Session> session);
	void BeginDisconnect(unsigned place);
	void BeginDisconnect(shared_ptr<srv::Session> session);
	void BeginDisconnect(srv::Session* session);

	shared_ptr<srv::Session> SeekNewbiePlace() const noexcept;
	unsigned long long MakeNewbieID() noexcept;

	int SendTo(srv::Session* session, void* const data, const std::unsigned_integral auto size);
	int SendServerStatus(srv::Session* session);
	int SendLoginResult(srv::Session* session, login_succeed_t info);
	int SendLoginResult(srv::Session* session, login_failure_t info);

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;

	unsigned int concurrentsNumber;
	std::latch concurrentWatcher;
	std::vector<Thread> myWorkers;
	std::stop_source workersBreaker;

	unique_ptr<Thread> timerWorker;
	std::priority_queue<TimedJob> timerQueue;

	unique_ptr<Thread> databaseWorker;
	std::unordered_set<DatabaseJob> databaseQueue;
	std::packaged_task<bool(const Sentence user_id)> databaseUserSearcher;
	std::packaged_task<bool(const Sentence user_id, const Sentence user_pw)> databaseUserCertifier;

	std::array<shared_ptr<srv::Room>, srv::MAX_ROOMS> everyRooms;
	std::array<shared_ptr<srv::Session>, srv::MAX_ENTITIES> everySessions;
	atomic<unsigned> numberRooms;
	atomic<unsigned> numberUsers;

	atomic<unsigned long long> playerIDs;
	srv::Protocol lastPacketType;
};

class TimedJob
{
public:
	constexpr TimedJob() = default;

	let bool operator==(const TimedJob& rhs) const noexcept;
	let bool operator<(const TimedJob& rhs) const noexcept;
};

class DatabaseJob
{
public:
	constexpr DatabaseJob() = default;

	let bool operator==(const DatabaseJob& rhs) const noexcept;
	let bool operator<(const DatabaseJob& rhs) const noexcept;
};

namespace srv
{
	template<packets Pk>
	let auto CreateTicket(const Pk& datagram)
	{
		Asynchron* asyncron = CreateAsynchron(Operations::SEND);

		auto packet = srv::CreatePacket(datagram);

		WSABUF wbuffer{};
		wbuffer.buf = reinterpret_cast<char*>(packet);
		wbuffer.len = packet->GetSize();

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}

	template<packets Pk>
	let auto CreateTicket(Pk&& datagram)
	{
		Asynchron* asyncron = CreateAsynchron(Operations::SEND);

		auto packet = srv::CreatePacket(std::forward<Pk>(datagram));

		WSABUF wbuffer{};
		wbuffer.buf = reinterpret_cast<char*>(packet);
		wbuffer.len = packet->GetSize();

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}

	template<packets Pk, typename ...Ty>
	let auto CreateTicket(Ty&& ...args)
	{
		Asynchron* asyncron = CreateAsynchron(Operations::SEND);

		auto packet = srv::CreatePacket<Pk>(std::forward<Ty>(args)...);

		WSABUF wbuffer{};
		wbuffer.buf = reinterpret_cast<char*>(packet);
		wbuffer.len = packet->GetSize();

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}
}
