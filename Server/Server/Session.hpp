#pragma once
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Room.hpp"

class Session
{
public:
	constexpr Session(unsigned place)
		: isFirst(false), mySwitch()
		, myPlace(place), mySocket(NULL), myID(0), myRoom(nullptr)
		, myReceiver(nullptr), myRecvBuffer(), myLastPacket()
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

	/// <summary>
	/// 수신한 버퍼의 내용을 정리하고 완성된 패킷은 반환합니다.
	/// </summary>
	/// <param name="size">작업이 끝나고 받을 바이트의 총량</param>
	/// <param name="bytes">수신받은 바이트의 수</param>
	inline std::optional<srv::BasisPacket*> Swallow(const unsigned size, _In_ const unsigned bytes)
	{
		Acquire();

		std::optional<srv::BasisPacket*> result{};

		auto& wbuffer = myReceiver->myBuffer;
		auto& cbuffer = wbuffer.buf;
		auto& cbuffer_length = wbuffer.len;

		// 패킷 조립
		constexpr auto min_size = sizeof(srv::BasisPacket);



		// 나머지 패킷을 수신
		const int op = Recv(size, static_cast<unsigned>(bytes));

		if (srv::CheckError(op)) [[unlikely]] {
			if (!srv::CheckPending(op)) [[unlikely]] {
				std::cout << "세션 " << myID.load(std::memory_order_relaxed) << "에서 수신 오류 발생!\n";

				BeginDisconnect();

				result.reset();
			}
		}
		Release();

		return result;
	}

	/// <summary>
	/// 비동기 연결 해제를 요청합니다. 나중에 Cleanup()를 호출하도록 합니다.
	/// </summary>
	inline void BeginDisconnect()
	{
		DisconnectEx(mySocket, srv::CreateAsynchron(srv::Operations::DISPOSE), 0, 0);
	}

	/// <summary>
	/// 접속 종료를 위해 세션이 속한 게임, 방, 대기실을 나갑니다.
	/// </summary>
	inline void BeginCleanup()
	{
		ZeroMemory(myLastPacket, sizeof(myLastPacket));
	}

	/// <summary>
	/// 세션의 내용을 초기화합니다. 실제 연결 해제를 수행하진 않습니다.
	/// </summary>
	inline void Cleanup()
	{
		AssignReceiveVirgin(true);
		AssignState(srv::SessionStates::NONE);
		AssignID(0);
		AssignSocket(NULL);
	}

	/// <summary>
	/// 세션의 소유권을 획득합니다.
	/// </summary>
	inline void Acquire() volatile
	{
		while (mySwitch.test_and_set(std::memory_order_acquire));
	}

	/// <summary>
	/// 한번 소유권 획득을 시도합니다.
	/// </summary>
	/// <returns>성공 여부</returns>
	inline bool TryAcquire() volatile
	{
		return !mySwitch.test_and_set(std::memory_order_acquire);
	}

	/// <summary>
	/// 소유권을 해제합니다.
	/// </summary>
	inline void Release() volatile
	{
		mySwitch.clear(std::memory_order_release);
	}

	/// <summary>
	/// 패킷 수신을 시작합니다.
	/// </summary>
	/// <returns>WSARecv의 결과값</returns>
	inline int BeginRecv()
	{
		myReceiver = make_shared<srv::Asynchron>(srv::Operations::RECV);
		myReceiver->SetBuffer(myRecvBuffer, 0); // Page 락을 줄이기 위해 맨 처음에 0으로 받음

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 세션 내부에서 수신을 처리합니다.
	/// </summary>
	/// <param name="size"></param>
	/// <param name="additional_offsets"></param>
	/// <returns>WSARecv의 결과값</returns>
	inline int Recv(unsigned size, std::integral auto additional_offsets)
	{
		auto& wbuffer = myReceiver->myBuffer;
		wbuffer.buf = (myRecvBuffer) + additional_offsets;
		wbuffer.len = static_cast<unsigned long>(size - additional_offsets);

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 세션 내부에서 수신을 처리합니다.
	/// </summary>
	/// <param name="size"></param>
	/// <returns>WSARecv의 결과값</returns>
	inline int Recv(unsigned size)
	{
		auto& wbuffer = myReceiver->myBuffer;
		wbuffer.buf = myRecvBuffer;
		wbuffer.len = static_cast<unsigned long>(size);

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 패킷 송신을 시작합니다.
	/// </summary>
	/// <param name="asynchron"></param>
	/// <returns>WSASend의 결과값</returns>
	inline int BeginSend(srv::Asynchron* asynchron)
	{
		return asynchron->Send(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 동기화 객체를 통해 송신을 처리합니다. 송신할 원본 자료가 필요합니다.
	/// </summary>
	/// <param name="asynchron"></param>
	/// <param name="buffer"></param>
	/// <param name="size"></param>
	/// <param name="offset"></param>
	/// <returns>WSASend의 결과값</returns>
	inline int Send(srv::Asynchron* asynchron, char* const buffer, unsigned size, unsigned offset = 0)
	{
		auto& wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 동기화 객체를 통해 송신을 처리합니다. 송신할 원본 자료가 필요합니다.
	/// </summary>
	/// <param name="asynchron"></param>
	/// <param name="buffer"></param>
	/// <param name="offset"></param>
	/// <returns>WSASend의 결과값</returns>
	template<unsigned original_size>
	inline int Send(srv::Asynchron* asynchron, const char(&buffer)[original_size], unsigned offset = 0)
	{
		auto& wbuffer = asynchron->myBuffer;
		wbuffer.buf = buffer + offset;
		wbuffer.len = static_cast<unsigned long>(original_size - offset);

		return asynchron->Send(mySocket, nullptr, 0);
	}

	/// <summary>
	/// 동기화 객체를 통해 송신을 처리합니다. 동기화 객체의 내부에서 버퍼의 위치를 조정합니다.
	/// </summary>
	/// <param name="asynchron"></param>
	/// <param name="additional_offsets"></param>
	/// <returns>WSASend의 결과값</returns>
	template<std::integral Integral>
	inline int Send(srv::Asynchron* asynchron, Integral additional_offsets)
	{
		auto& wbuffer = asynchron->myBuffer;
		wbuffer.buf += additional_offsets;
		wbuffer.len -= additional_offsets;

		return asynchron->Send(mySocket, nullptr, 0);
	}

	inline void AssignReceiveVirgin(const bool flag)
	{
		isFirst.store(flag, std::memory_order_acq_rel);
	}

	inline void AssignState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_acq_rel);
	}

	inline void AssignSocket(const SOCKET& sock)
	{
		mySocket.store(sock, std::memory_order_acq_rel);
	}

	inline void AssignSocket(SOCKET&& sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_acq_rel);
	}

	inline void AssignID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_acq_rel);
	}

	inline void AssignRoom(const shared_ptr<Room>& room)
	{
		myRoom.store(room, std::memory_order_acq_rel);
	}

	inline void AssignRoom(shared_ptr<Room>&& room)
	{
		myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_acq_rel);
	}

	inline void SetReceiveVirgin(const bool flag)
	{
		isFirst.store(flag, std::memory_order_relaxed);
	}

	inline void SetState(const srv::SessionStates state)
	{
		myState.store(state, std::memory_order_relaxed);
	}

	inline void SetSocket(const SOCKET& sock)
	{
		mySocket.store(sock, std::memory_order_relaxed);
	}

	inline void SetSocket(SOCKET&& sock)
	{
		mySocket.store(std::forward<SOCKET>(sock), std::memory_order_relaxed);
	}

	inline void SetID(const unsigned long long id)
	{
		myID.store(id, std::memory_order_relaxed);
	}

	inline void SetRoom(const shared_ptr<Room>& room)
	{
		myRoom.store(room, std::memory_order_relaxed);
	}

	inline void SetRoom(shared_ptr<Room>&& room)
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

	atomic<bool> isFirst;
	atomic_flag mySwitch;
	atomic<srv::SessionStates> myState;
	atomic<SOCKET> mySocket;
	atomic<unsigned long long> myID;
	atomic<shared_ptr<Room>> myRoom;

	shared_ptr<srv::Asynchron> myReceiver;
	char myRecvBuffer[BUFSIZ];
	char myLastPacket[200];
};
