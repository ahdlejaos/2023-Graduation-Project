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
	/// ������ �ʱ�ȭ�� �����մϴ�.
	/// </summary>
	/// <param name="id"></param>
	/// <param name="sock">���ο� ����</param>
	inline void Ready(unsigned long long id, SOCKET sock)
	{
		AssignState(srv::SessionStates::ACCEPTED);
		AssignID(id);
		AssignSocket(sock);
	}

	/// <summary>
	/// ������ ���¸� ��������� �ٲߴϴ�. ������ ������ ���������� �ʽ��ϴ�.
	/// </summary>
	inline void Connect()
	{
		AssignState(srv::SessionStates::CONNECTED);
	}

	/// <summary>
	/// ������ ������ ������ �����ϰ� �ϼ��� ��Ŷ�� ��ȯ�մϴ�.
	/// </summary>
	/// <param name="size">�۾��� ������ ���� ����Ʈ�� �ѷ�</param>
	/// <param name="bytes">���Ź��� ����Ʈ�� ��</param>
	inline std::optional<srv::BasisPacket*> Swallow(const unsigned size, _In_ const unsigned bytes)
	{
		Acquire();

		std::optional<srv::BasisPacket*> result{};

		auto& wbuffer = myReceiver->myBuffer;
		auto& cbuffer = wbuffer.buf;
		auto& cbuffer_length = wbuffer.len;

		// ��Ŷ ����
		constexpr auto min_size = sizeof(srv::BasisPacket);



		// ������ ��Ŷ�� ����
		const int op = Recv(size, static_cast<unsigned>(bytes));

		if (srv::CheckError(op)) [[unlikely]] {
			if (!srv::CheckPending(op)) [[unlikely]] {
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
		DisconnectEx(mySocket, srv::CreateAsynchron(srv::Operations::DISPOSE), 0, 0);
	}

	/// <summary>
	/// ���� ���Ḧ ���� ������ ���� ����, ��, ������ �����ϴ�.
	/// </summary>
	inline void BeginCleanup()
	{
		ZeroMemory(myLastPacket, sizeof(myLastPacket));
	}

	/// <summary>
	/// ������ ������ �ʱ�ȭ�մϴ�. ���� ���� ������ �������� �ʽ��ϴ�.
	/// </summary>
	inline void Cleanup()
	{
		AssignReceiveVirgin(true);
		AssignState(srv::SessionStates::NONE);
		AssignID(0);
		AssignSocket(NULL);
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
		myReceiver = make_shared<srv::Asynchron>(srv::Operations::RECV);
		myReceiver->SetBuffer(myRecvBuffer, 0); // Page ���� ���̱� ���� �� ó���� 0���� ����

		return myReceiver->Recv(mySocket, nullptr, 0);
	}

	/// <summary>
	/// ���� ���ο��� ������ ó���մϴ�.
	/// </summary>
	/// <param name="size"></param>
	/// <param name="additional_offsets"></param>
	/// <returns>WSARecv�� �����</returns>
	inline int Recv(unsigned size, std::integral auto additional_offsets)
	{
		auto& wbuffer = myReceiver->myBuffer;
		wbuffer.buf = (myRecvBuffer) + additional_offsets;
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

	/// <summary>
	/// ��Ŷ �۽��� �����մϴ�.
	/// </summary>
	/// <param name="asynchron"></param>
	/// <returns>WSASend�� �����</returns>
	inline int BeginSend(srv::Asynchron* asynchron)
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
	inline int Send(srv::Asynchron* asynchron, char* const buffer, unsigned size, unsigned offset = 0)
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
	inline int Send(srv::Asynchron* asynchron, const char(&buffer)[original_size], unsigned offset = 0)
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
