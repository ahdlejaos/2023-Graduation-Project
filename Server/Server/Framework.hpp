#pragma once
#include "AsyncPoolService.hpp"
#include "ConnectService.hpp"
#include "Packet.hpp"
#include "Asynchron.hpp"

template<>
struct std::hash<DatabaseJob>
{
	[[nodiscard]] size_t operator()(const DatabaseJob& _Keyval) const noexcept
	{
		return _Hash_representation(reinterpret_cast<const void*>(std::addressof(_Keyval)));
	}
};

enum class TIMED_JOB_TYPES : unsigned char
{
	NONE = 0,

};

enum class DB_JOB_TYPES : unsigned char
{
	NONE = 0,
	CHECK_ID,
	FIND_USER,
};

class Framework
{
private:
#pragma region Prefabs
	struct login_succeed_t
	{};

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
	friend void TimerWorker(std::stop_source& stopper, Framework& me);
	friend void DBaseWorker(std::stop_source& stopper, Framework& me);

	bool CanAcceptPlayer() const noexcept;
	bool CanCreateRoom() const noexcept;

	shared_ptr<srv::Session> GetSession(unsigned place) const noexcept(false);
	shared_ptr<srv::Session> FindSession(unsigned long long id) const noexcept(false);

	template<typename Ty, typename ...RestTy>
	constexpr void Print(Ty&& first, RestTy&& ...rests);

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
	int SendLoginResult(srv::Session* session, const login_succeed_t& info);
	int SendLoginResult(srv::Session* session, const login_failure_t& info);

	template<typename Ty, typename ...RestTy>
	constexpr void UnsafePrint(Ty&& first, RestTy&& ...rests);

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	AsyncPoolService myAsyncProvider;

	unsigned int concurrentsNumber;
	std::latch concurrentWatcher;
	std::vector<Thread> myWorkers;
	std::stop_source workersBreaker;
	std::atomic_flag concurrentOutputLock;

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

	atomic<PID> playerIDs;
	srv::Protocol lastPacketType;
};

class TimedJob
{
public:
	constexpr TimedJob(TIMED_JOB_TYPES type, Clock time)
		: myType(type), myTime(time)
		, myData()
		, myReturn(nullptr)
	{}

	let void SetData(const std::span<char, 100> data) noexcept(false)
	{
		std::copy(data.begin(), data.end(), std::begin(myData));
	}

	let void SetData(char* data, const std::integral auto offset) noexcept(false)
	{
		std::copy(data.begin(), data.end(), std::advance(std::begin(myData), offset));
	}

	let bool operator==(const TimedJob& rhs) const noexcept
	{
		return (this == std::addressof(rhs));
	}

	let std::strong_ordering operator<=>(const TimedJob& rhs) const noexcept
	{
		return myTime <=> rhs.myTime;
	}

	Clock myTime;
	TIMED_JOB_TYPES myType;
	char myData[100];
	void* myReturn;
};

class DatabaseJob
{
public: 
	constexpr DatabaseJob(DB_JOB_TYPES type)
		: myType(type)
		, myData(), myReturn(nullptr)
	{}

	let void SetData(const std::span<char, 100> data) noexcept(false)
	{
		std::copy(data.begin(), data.end(), std::begin(myData));
	}

	let void SetData(char* data, const std::integral auto offset) noexcept(false)
	{
		std::copy(data.begin(), data.end(), std::advance(std::begin(myData), offset));
	}

	let bool operator==(const DatabaseJob& rhs) const noexcept
	{
		return (this == std::addressof(rhs));
	}

	let std::strong_ordering operator<=>(const DatabaseJob& rhs) const noexcept
	{
		return this <=> std::addressof(rhs);
	}

	DB_JOB_TYPES myType;
	char myData[100];
	void* myReturn;
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

template<typename Ty, typename ...RestTy>
constexpr void Framework::Print(Ty&& first, RestTy&& ...rests)
{
	while (concurrentOutputLock.test_and_set(std::memory_order_acquire));
	std::cout << std::forward<Ty>(first);

	if constexpr(0 < sizeof...(RestTy))
	{
		UnsafePrint(std::forward<RestTy>(rests)...);
	}
	concurrentOutputLock.clear(std::memory_order_release);
}

template<typename Ty, typename ...RestTy>
constexpr void Framework::UnsafePrint(Ty&& first, RestTy&& ...rests)
{
	std::cout << std::forward<Ty>(first);

	if constexpr (0 < sizeof...(RestTy))
	{
		UnsafePrint(std::forward<RestTy>(rests)...);
	}
}
