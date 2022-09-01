#pragma once
#include "Asynchron.hpp"
#include "Packet.hpp"
#include "Room.hpp"

namespace srv
{
	class Session : public std::enable_shared_from_this<Session>
	{
	private:
		constexpr Session(unsigned place)
			: isFirst(false), mySwitch()
			, myPlace(place), mySocket(NULL), myID(0), myRoom(nullptr)
			, myReceiver(nullptr), myRecvBuffer(), myRecvSize(), myLastPacket()
		{}

	public:
		virtual ~Session()
		{}

		[[nodiscard]] inline static shared_ptr<Session> Create(unsigned place) noexcept
		{
			return shared_ptr<Session>(new Session{ place });
		}

		inline shared_ptr<Session> Handle() noexcept
		{
			return shared_from_this();
		}

		/// <summary>
		/// 세션의 초기화를 수행합니다.
		/// </summary>
		/// <param name="id"></param>
		/// <param name="sock">새로운 소켓</param>
		inline void Ready(const PID id, SOCKET sock)
		{
			AssignState(SessionStates::ACCEPTED);
			AssignID(id);
			AssignSocket(sock);
		}

		/// <summary>
		/// 세션의 상태를 연결됨으로 바꿉니다. 실제로 연결을 수행하지는 않습니다.
		/// </summary>
		inline void Connect()
		{
			AssignState(SessionStates::CONNECTED);
		}

		/// <summary>
		/// 수신한 버퍼의 내용을 정리하고 완성된 패킷은 반환합니다.
		/// </summary>
		/// <param name="size">작업이 끝나고 받을 바이트의 총량</param>
		/// <param name="bytes">수신받은 바이트의 수</param>
		inline std::optional<BasisPacket*> Swallow(const unsigned size, _In_ const unsigned bytes)
		{
			Acquire();

			std::optional<BasisPacket*> result{};

			auto& wbuffer = myReceiver->myBuffer;
			auto& cbuffer = wbuffer.buf;
			auto& cbuffer_length = wbuffer.len;

			// 받은 바이트 수 증가
			myRecvSize += bytes;

			// 패킷 조립
			constexpr unsigned min_size = sizeof(BasisPacket);

			if (min_size <= myRecvSize)
			{
				// 받은 버퍼를 패킷으로 해석
				const auto& pk_proto = reinterpret_cast<BasisPacket*>(cbuffer);
				const auto& pk_size = pk_proto->GetSize();

				if (pk_size <= myRecvSize)
				{
					result = pk_proto;

					myRecvSize -= pk_size;
				}
				else
				{
					std::cout << "세션 " << myID.load(std::memory_order_relaxed) << "에서 패킷을 조립하지 못했습니다.\n";
				}
			}
			else
			{
				std::cout << "세션 " << myID.load(std::memory_order_relaxed) << "에서 최소 요건이 맞지 않아서 패킷을 조립하지 못했습니다.\n";
			}

			// 나머지 패킷을 수신
			const int op = Recv(size, myRecvSize);

			if (CheckError(op)) [[unlikely]] {
				if (!CheckPending(op)) [[unlikely]] {
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
			DisconnectEx(mySocket, CreateAsynchron(Operations::DISPOSE), 0, 0);
		}

		/// <summary>
		/// 접속 종료를 위해 세션이 속한 게임, 방, 대기실을 나갑니다.
		/// </summary>
		let void BeginCleanup()
		{
			Clear(myLastPacket, 0);
		}

		/// <summary>
		/// 세션의 내용을 초기화합니다. 실제 연결 해제를 수행하진 않습니다.
		/// </summary>
		inline void Cleanup()
		{
			AssignReceiveVirgin(true);
			AssignState(SessionStates::NONE);
			AssignID(0);
			AssignSocket(NULL);

			myRecvSize = 0;
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
			myReceiver = make_shared<Asynchron>(Operations::RECV);
			myReceiver->SetBuffer(myRecvBuffer, 0); // Page 락을 줄이기 위해 맨 처음에 0으로 받음

			return myReceiver->Recv(mySocket, nullptr, 0);
		}

		/// <summary>
		/// 세션 내부에서 수신을 처리합니다.
		/// </summary>
		/// <param name="size"></param>
		/// <param name="additional_offsets"></param>
		/// <returns>WSARecv의 결과값</returns>
		inline int Recv(unsigned size, const std::integral auto additional_offsets)
		{
			auto& wbuffer = myReceiver->myBuffer;
			wbuffer.buf = myRecvBuffer + additional_offsets;
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

		inline constexpr void ClearRecvBuffer(unsigned begin_offset = 0)
		{
			std::fill(std::begin(myRecvBuffer) + begin_offset, std::end(myRecvBuffer), 0);
		}

		/// <summary>
		/// 패킷 송신을 시작합니다.
		/// </summary>
		/// <param name="asynchron"></param>
		/// <returns>WSASend의 결과값</returns>
		inline int BeginSend(Asynchron* asynchron)
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
		inline int Send(Asynchron* asynchron, char* const buffer, unsigned size, unsigned offset = 0)
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
		inline int Send(Asynchron* asynchron, const char(&buffer)[original_size], unsigned offset = 0)
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
		inline int Send(Asynchron* asynchron, std::integral auto additional_offsets)
		{
			auto& wbuffer = asynchron->myBuffer;
			wbuffer.buf += additional_offsets;
			wbuffer.len -= additional_offsets;

			return asynchron->Send(mySocket, nullptr, 0);
		}

		inline void AssignReceiveVirgin(const bool flag)
		{
			isFirst.store(flag, std::memory_order_release);
		}

		inline void AssignState(const SessionStates state)
		{
			myState.store(state, std::memory_order_release);
		}

		inline void AssignSocket(const SOCKET& sock)
		{
			mySocket.store(sock, std::memory_order_release);
		}

		inline void AssignSocket(SOCKET&& sock)
		{
			mySocket.store(std::forward<SOCKET>(sock), std::memory_order_release);
		}

		inline void AssignID(const PID id)
		{
			myID.store(id, std::memory_order_release);
		}

		inline void AssignRoom(const shared_ptr<Room>& room)
		{
			myRoom.store(room, std::memory_order_release);
		}

		inline void AssignRoom(shared_ptr<Room>&& room)
		{
			myRoom.store(std::forward<shared_ptr<Room>>(room), std::memory_order_release);
		}

		inline void SetReceiveVirgin(const bool flag)
		{
			isFirst.store(flag, std::memory_order_relaxed);
		}

		inline void SetState(const SessionStates state)
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

		inline void SetID(const PID id)
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

		inline constexpr virtual bool IsUser() const noexcept
		{
			return false;
		}

		inline constexpr virtual bool IsNotUser() const noexcept
		{
			return true;
		}

		const unsigned int myPlace;

		atomic<bool> isFirst;
		atomic_flag mySwitch;
		atomic<SessionStates> myState;
		atomic<SOCKET> mySocket;
		atomic<PID> myID;
		atomic<shared_ptr<Room>> myRoom;

		shared_ptr<Asynchron> myReceiver;
		char myRecvBuffer[BUFSIZ];
		unsigned myRecvSize;
		char myLastPacket[200];
	};
}
