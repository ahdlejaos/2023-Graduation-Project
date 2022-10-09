#pragma once
#include "ConnectService.hpp"
#include "DatabaseService.hpp"
#include "DatabaseQuery.hpp"
#include "Packet.hpp"
#include "Asynchron.hpp"
#include "Spinlock.inl"

template<>
struct std::hash<db::Job>
{
	[[nodiscard]] size_t operator()(const db::Job& _Keyval) const noexcept
	{
		return _Hash_representation(reinterpret_cast<const void*>(std::addressof(_Keyval)));
	}
};

struct TimerBlob;

using SessionPtr = srv::Session*;
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

	BOOL PostDatabaseJob(const PID user_id, const DWORD data);
	BOOL PostDatabaseJob(const PID user_id, const srv::DatabaseTasks type, void* blob);
	db::Query& DBAddPlayer(BasicUserBlob data);
	db::Query& DBFindPlayer(const std::wstring_view& email);
	db::Query& DBFindPlayerByNickname(const std::wstring_view& nickname);
	db::Query& DBUpdatePlayer(const PID id, BasicUserBlob data);

	void RouteSucceed(LPWSAOVERLAPPED context, ULONG_PTR key, unsigned bytes);
	void RouteFailed(LPWSAOVERLAPPED context, ULONG_PTR key, unsigned bytes);

	void ProceedAccept(srv::Asynchron* context);
	void ProceedSent(srv::Asynchron* context, ULONG_PTR key, unsigned bytes);
	void ProceedRecv(srv::Asynchron* context, ULONG_PTR key, unsigned bytes);
	void ProceedDispose(srv::Asynchron* context, ULONG_PTR key);

	void ProceedBeginDiconnect(ULONG_PTR key);
	void ProceedBeginDiconnect(SessionPtr session);

	friend void Worker(std::stop_source& stopper, Framework& me, ConnectService& pool);
	friend void TimerWorker(std::stop_source& stopper, Framework& me);
	friend void DBaseWorker(std::stop_source& stopper, Framework& me);

	bool CanAcceptPlayer() const noexcept;
	bool CanCreateRoom() const noexcept;

	SessionPtr GetSession(const std::size_t place) const noexcept(false);
	SessionPtr FindSession(const PID id) const noexcept(false);

	template<typename Ty, typename ...RestTy>
	constexpr void Print(Ty&& first, RestTy&& ...rests);
	template<typename Ty, typename ...RestTy>
	constexpr void Println(Ty&& first, RestTy&& ...rests);

	using DBinUsersSearcher = bool(const Sentence user_id);

private:
	void BuildDatabase();
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	SessionPtr AcceptPlayer(SOCKET target);
	SessionPtr ConnectPlayer(const std::size_t place);
	SessionPtr ConnectPlayer(SessionPtr session);
	void BeginDisconnect(const std::size_t place);
	void BeginDisconnect(SessionPtr session);

	int SendTo(SessionPtr session, void* const data, const std::unsigned_integral auto size);
	int SendServerStatus(SessionPtr session);
	int SendLoginResult(SessionPtr session, const login_succeed_t& info);
	int SendLoginResult(SessionPtr session, const login_failure_t& info);

	srv::Session* SeekNewbiePlace() const noexcept;
	unsigned long long MakeNewbieID() noexcept;

	template<typename Ty, typename ...RestTy>
	constexpr void UnsafePrint(Ty&& first, RestTy&& ...rests);

	ULONG_PTR myID;

	ConnectService myEntryPoint;
	db::Service myDatabaseService;

	unsigned int concurrentsNumber;
	std::latch concurrentWatcher;
	std::vector<Thread> myWorkers;
	std::stop_source workersBreaker;
	Spinlock concurrentOutputLock;

	unique_ptr<Thread> timerWorker;
	std::priority_queue<TimedJob> timerQueue;

	unique_ptr<Thread> databaseWorker;
	srv::Asynchron databaseAsyncer;

	std::array<srv::Room*, srv::MAX_ROOMS> everyRooms;
	std::array<SessionPtr, srv::MAX_ENTITIES> everySessions;
	std::array<SessionPtr, srv::MAX_USERS> lobbySessions;
	std::unordered_map<PID, SessionPtr> dictSessions;
	atomic<unsigned> numberRooms;
	atomic<unsigned> numberUsers;

	atomic<PID> playerIDs;
	srv::Protocol lastPacketType;
};

class TimedJob
{
public:
	constexpr TimedJob(srv::DatabaseTasks type, Clock time)
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

protected:
	Clock myTime;
	srv::DatabaseTasks myType;
	char myData[100];
	void* myReturn;
};

struct BasicUserBlob
{
	PID id;
	wchar_t email[20];
	wchar_t nickname[16];
	wchar_t password[16];
};

struct UserBlob : BasicUserBlob
{
	unsigned int win, lose;
	unsigned long level, exp;
};

struct TimerBlob
{
	DWORD info_bytes;
	ULONG_PTR info_key;
	LPWSAOVERLAPPED info_overlapped = nullptr;
};

namespace srv
{
	template<packets Pk>
	let auto CreateTicket(const Pk& datagram)
	{
		Asynchron* asyncron = CreateAsynchron(Operations::SEND);

		Pk* packet = srv::CreatePacket(datagram);

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

		Pk* packet = srv::CreatePacket(std::forward<Pk>(datagram));

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

		Pk* packet = srv::CreatePacket<Pk>(std::forward<Ty>(args)...);

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
	concurrentOutputLock.lock();

	std::cout << std::forward<Ty>(first);

	if constexpr(0 < sizeof...(RestTy))
	{
		UnsafePrint(std::forward<RestTy>(rests)...);
	}

	concurrentOutputLock.unlock();
}

template<typename Ty, typename ...RestTy>
constexpr void Framework::Println(Ty&& first, RestTy&& ...rests)
{
	concurrentOutputLock.lock();

	std::cout << std::forward<Ty>(first);

	if constexpr (0 < sizeof...(RestTy))
	{
		UnsafePrint(std::forward<RestTy>(rests)...);
	}

	std::cout << '\n';

	concurrentOutputLock.unlock();
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
