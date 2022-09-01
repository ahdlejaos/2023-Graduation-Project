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
		/// ������ �ʱ�ȭ�� �����մϴ�.
		/// </summary>
		/// <param name="id"></param>
		/// <param name="sock">���ο� ����</param>
		inline void Ready(const PID id, SOCKET sock)
		{
			AssignState(SessionStates::ACCEPTED);
			AssignID(id);
			AssignSocket(sock);
		}

		/// <summary>
		/// ������ ���¸� ��������� �ٲߴϴ�. ������ ������ ���������� �ʽ��ϴ�.
		/// </summary>
		inline void Connect()
		{
			AssignState(SessionStates::CONNECTED);
		}

		/// <summary>
		/// ������ ������ ������ �����ϰ� �ϼ��� ��Ŷ�� ��ȯ�մϴ�.
		/// </summary>
		/// <param name="size">�۾��� ������ ���� ����Ʈ�� �ѷ�</param>
		/// <param name="bytes">���Ź��� ����Ʈ�� ��</param>
		inline std::optional<BasisPacket*> Swallow(const unsigned size, _In_ const unsigned bytes)
		{
			Acquire();

			std::optional<BasisPacket*> result{};

			auto& wbuffer = myReceiver->myBuffer;
			auto& cbuffer = wbuffer.buf;
			auto& cbuffer_length = wbuffer.len;

			// ���� ����Ʈ �� ����
			myRecvSize += bytes;

			// ��Ŷ ����
			constexpr unsigned min_size = sizeof(BasisPacket);

			if (min_size <= myRecvSize)
			{
				// ���� ���۸� ��Ŷ���� �ؼ�
				const auto& pk_proto = reinterpret_cast<BasisPacket*>(cbuffer);
				const auto& pk_size = pk_proto->GetSize();

				if (pk_size <= myRecvSize)
				{
					result = pk_proto;

					myRecvSize -= pk_size;
				}
				else
				{
					std::cout << "���� " << myID.load(std::memory_order_relaxed) << "���� ��Ŷ�� �������� ���߽��ϴ�.\n";
				}
			}
			else
			{
				std::cout << "���� " << myID.load(std::memory_order_relaxed) << "���� �ּ� ����� ���� �ʾƼ� ��Ŷ�� �������� ���߽��ϴ�.\n";
			}

			// ������ ��Ŷ�� ����
			const int op = Recv(size, myRecvSize);

			if (CheckError(op)) [[unlikely]] {
				if (!CheckPending(op)) [[unlikely]] {
					std::cout << "���� " << myID.load(std::memory_order_relaxed) << "���� ���� ���� �߻�!\n";

					BeginDisconnect();

					result.reset();
				}
			}
			Release();

			return result;
		}

		/// <summary>
		/// �񵿱� ���� ������ ��û�մϴ�. ���߿� Cleanup()�� ȣ���ϵ��� �մϴ�.
		/// </summary>
		inline void BeginDisconnect()
		{
			DisconnectEx(mySocket, CreateAsynchron(Operations::DISPOSE), 0, 0);
		}

		/// <summary>
		/// ���� ���Ḧ ���� ������ ���� ����, ��, ������ �����ϴ�.
		/// </summary>
		let void BeginCleanup()
		{
			Clear(myLastPacket, 0);
		}

		/// <summary>
		/// ������ ������ �ʱ�ȭ�մϴ�. ���� ���� ������ �������� �ʽ��ϴ�.
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
		/// ������ �������� ȹ���մϴ�.
		/// </summary>
		inline void Acquire() volatile
		{
			while (mySwitch.test_and_set(std::memory_order_acquire));
		}

		/// <summary>
		/// �ѹ� ������ ȹ���� �õ��մϴ�.
		/// </summary>
		/// <returns>���� ����</returns>
		inline bool TryAcquire() volatile
		{
			return !mySwitch.test_and_set(std::memory_order_acquire);
		}

		/// <summary>
		/// �������� �����մϴ�.
		/// </summary>
		inline void Release() volatile
		{
			mySwitch.clear(std::memory_order_release);
		}

		/// <summary>
		/// ��Ŷ ������ �����մϴ�.
		/// </summary>
		/// <returns>WSARecv�� �����</returns>
		inline int BeginRecv()
		{
			myReceiver = make_shared<Asynchron>(Operations::RECV);
			myReceiver->SetBuffer(myRecvBuffer, 0); // Page ���� ���̱� ���� �� ó���� 0���� ����

			return myReceiver->Recv(mySocket, nullptr, 0);
		}

		/// <summary>
		/// ���� ���ο��� ������ ó���մϴ�.
		/// </summary>
		/// <param name="size"></param>
		/// <param name="additional_offsets"></param>
		/// <returns>WSARecv�� �����</returns>
		inline int Recv(unsigned size, const std::integral auto additional_offsets)
		{
			auto& wbuffer = myReceiver->myBuffer;
			wbuffer.buf = myRecvBuffer + additional_offsets;
			wbuffer.len = static_cast<unsigned long>(size - additional_offsets);

			return myReceiver->Recv(mySocket, nullptr, 0);
		}

		/// <summary>
		/// ���� ���ο��� ������ ó���մϴ�.
		/// </summary>
		/// <param name="size"></param>
		/// <returns>WSARecv�� �����</returns>
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
		/// ��Ŷ �۽��� �����մϴ�.
		/// </summary>
		/// <param name="asynchron"></param>
		/// <returns>WSASend�� �����</returns>
		inline int BeginSend(Asynchron* asynchron)
		{
			return asynchron->Send(mySocket, nullptr, 0);
		}

		/// <summary>
		/// ����ȭ ��ü�� ���� �۽��� ó���մϴ�. �۽��� ���� �ڷᰡ �ʿ��մϴ�.
		/// </summary>
		/// <param name="asynchron"></param>
		/// <param name="buffer"></param>
		/// <param name="size"></param>
		/// <param name="offset"></param>
		/// <returns>WSASend�� �����</returns>
		inline int Send(Asynchron* asynchron, char* const buffer, unsigned size, unsigned offset = 0)
		{
			auto& wbuffer = asynchron->myBuffer;
			wbuffer.buf = buffer + offset;
			wbuffer.len = static_cast<unsigned long>(size - offset);

			return asynchron->Send(mySocket, nullptr, 0);
		}

		/// <summary>
		/// ����ȭ ��ü�� ���� �۽��� ó���մϴ�. �۽��� ���� �ڷᰡ �ʿ��մϴ�.
		/// </summary>
		/// <param name="asynchron"></param>
		/// <param name="buffer"></param>
		/// <param name="offset"></param>
		/// <returns>WSASend�� �����</returns>
		template<unsigned original_size>
		inline int Send(Asynchron* asynchron, const char(&buffer)[original_size], unsigned offset = 0)
		{
			auto& wbuffer = asynchron->myBuffer;
			wbuffer.buf = buffer + offset;
			wbuffer.len = static_cast<unsigned long>(original_size - offset);

			return asynchron->Send(mySocket, nullptr, 0);
		}

		/// <summary>
		/// ����ȭ ��ü�� ���� �۽��� ó���մϴ�. ����ȭ ��ü�� ���ο��� ������ ��ġ�� �����մϴ�.
		/// </summary>
		/// <param name="asynchron"></param>
		/// <param name="additional_offsets"></param>
		/// <returns>WSASend�� �����</returns>
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
