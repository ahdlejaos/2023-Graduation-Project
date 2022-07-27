#pragma once
#include "Asynchron.hpp"
#include "Room.hpp"

class Session
{
public:
	constexpr Session(unsigned place)
		: mySwitch()
		, myPlace(place), mySocket(NULL), myID(0), myRoom(nullptr)
		, myReceiver(nullptr), myRecvBuffer()
	{}

	virtual ~Session()
	{}

	/// <summary>
	/// 세션의 초기화를 수행합니다.
	/// </summary>
	/// <param name="id"></param>
	/// <param name="sock">새로운 소켓</param>
	inline void Ready(unsigned long long id, SOCKET sock)
	{
		AssignState(srv::SessionStates::ACCEPTED);
		AssignID(id);
		AssignSocket(sock);
	}

	/// <summary>
	/// 세션의 상태를 연결됨으로 바꿉니다. 실제로 연결을 수행하지는 않습니다.
	/// </summary>
	inline void Connect()
	{
		AssignState(srv::SessionStates::CONNECTED);
	}

	inline void Swallow(unsigned size, unsigned bytes)
	{


		// 나머지 패킷을 수신
		Recv(size, static_cast<unsigned>(bytes));
	}

	/// <summary>
	/// 비동기 연결 해제를 요청합니다. 나중에 Dispose()를 호출하도록 합니다.
	/// </summary>
	inline void Disconnect()
	{
		DisconnectEx(mySocket, srv::CreateAsynchron(srv::Operations::DISPOSE), 0, 0);
	}

	/// <summary>
	/// 세션 정리
	/// </summary>
	inline void Dispose()
	{
		AssignState(srv::SessionStates::NONE);
		AssignID(0);
		AssignSocket(NULL);
	}

	inline void Acquire() volatile
	{
		while (mySwitch.test_and_set(std::memory_order_acquire));
	}

	inline bool TryAcquire() volatile
	{
		return !mySwitch.test_and_set(std::memory_order_acquire);
	}

	inline void Release() volatile
	{
		mySwitch.clear(std::memory_order_release);
	}

	inline int BeginSend(Asynchron *asynchron)
	{
		return asynchron->Send(mySocket, nullptr, 0);
	}

	inline int BeginRecv()
	{
		myReceiver = make_shared<Asynchron>(srv::Operations::RECV);
		myReceiver->SetBuffer(myRecvBuffer, 0); // Page 락을 줄이기 위해 맨 처음에 0으로 받음

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	inline int Send(Asynchron *asynchron, char *const buffer, unsigned size, unsigned offset = 0)
	{
		auto &wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	template<unsigned original_size>
	inline int Send(Asynchron *asynchron, const char(&buffer)[original_size], unsigned offset = 0)
	{
		auto &wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(original_size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	template<std::unsigned_integral Integral>
	inline int Send(Asynchron *asynchron, Integral additional_offsets)
	{
		auto &wbuffer = asynchron->myBuffer;
		wbuffer.buf += additional_offsets;
		wbuffer.len -= additional_offsets;

		return asynchron->Send(mySocket, nullptr, 0);
	}

	inline int Recv(unsigned size, unsigned offset = 0)
	{
		auto &wbuffer = myReceiver->myBuffer;
		wbuffer.buf = (myRecvBuffer)+offset;
		wbuffer.len = static_cast<unsigned long>(size - offset);

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	inline void AssignState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_acq_rel);
	}

	inline void AssignSocket(const SOCKET &sock)
	{
		mySocket.store(sock, std::memory_order_acq_rel);
	}

	inline void AssignSocket(SOCKET &&sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_acq_rel);
	}

	inline void AssignID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_acq_rel);
	}

	inline void AssignRoom(const shared_ptr<Room> &room)
	{
		myRoom.store(room, std::memory_order_acq_rel);
	}

	inline void AssignRoom(shared_ptr<Room> &&room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_acq_rel);
	}

	inline void SetState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_relaxed);
	}

	inline void SetSocket(const SOCKET &sock)
	{
		mySocket.store(sock, std::memory_order_relaxed);
	}

	inline void SetSocket(SOCKET &&sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_relaxed);
	}

	inline void SetID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_relaxed);
	}

	inline void SetRoom(const shared_ptr<Room> &room)
	{
		myRoom.store(room, std::memory_order_relaxed);
	}

	inline void SetRoom(shared_ptr<Room> &&room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_relaxed);
	}

	inline constexpr virtual bool IsUser() noexcept
	{
		return false;
	}

	inline constexpr virtual bool IsNotUser() noexcept
	{
		return true;
	}

	const unsigned int myPlace;

	atomic_flag mySwitch;
	atomic<srv::SessionStates> myState;
	atomic<SOCKET> mySocket;
	atomic<unsigned long long> myID;
	atomic<shared_ptr<Room>> myRoom;

	shared_ptr<Asynchron> myReceiver;
	char myRecvBuffer[BUFSIZ];
};
