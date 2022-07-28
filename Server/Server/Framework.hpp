#pragma once
#include "AsyncPoolService.hpp"
#include "ConnectService.hpp"
#include "Packet.hpp"

class Framework
{
private:
#pragma region ParamScrews
	struct login_succeed_t { srv::SIGNIN_CAUSE cause; };
	struct login_failure_t { srv::SIGNIN_CAUSE cause; };

	login_succeed_t login_succeed;
	login_failure_t login_failure;
#pragma endregion

public:
	Framework();
	~Framework();

	void Awake(unsigned int concurrent_hint, unsigned short port_tcp);
	void Start();
	void Update();
	void Release();

	void ProceedAsync(Asynchron *context, ULONG_PTR key, unsigned bytes);
	void ProceedAccept(Asynchron *context);
	void ProceedDiconnect(Asynchron *context, ULONG_PTR key);
	void ProceedSent(Asynchron *context, ULONG_PTR key, unsigned bytes);
	void ProceedRecv(Asynchron *context, ULONG_PTR key, unsigned bytes);
	void ProceedDispose(Asynchron *context, ULONG_PTR key);

	friend void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool);

	shared_ptr<Session> GetSession(unsigned place) const noexcept(false);

private:
	void BuildSessions();
	void BuildRooms();
	void BuildResources();

	shared_ptr<Session> AcceptPlayer(SOCKET target);
	shared_ptr<Session> ConnectPlayer(unsigned place);
	shared_ptr<Session> ConnectPlayer(shared_ptr<Session> session);
	void Disconnect(unsigned place);
	void Disconnect(shared_ptr<Session> session);
	void Disconnect(Session *session);

	shared_ptr<Session> SeekNewbiePlace() const noexcept;
	unsigned long long MakeNewbieID() noexcept;

	template<std::unsigned_integral Integral>
	int SendTo(Session *session, void *const data, const Integral size);
	int SendServerStatus(Session *session);
	int SendLoginResult(Session *session, login_succeed_t info);
	int SendLoginResult(Session *session, login_failure_t info);

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
	inline std::pair<PACKET *, Asynchron *> CreateTicket(Ty&& ...args)
	{
		Asynchron *asyncron = CreateAsynchron(Operations::SEND);

		auto packet = srv::CreatePacket<PACKET>(std::forward<decltype(args)>(args)...);

		WSABUF wbuffer{};
		wbuffer.buf = reinterpret_cast<char *>(packet);
		wbuffer.len = packet->mySize;

		asyncron->SetBuffer(wbuffer);

		return make_pair(packet, asyncron);
	}
}
