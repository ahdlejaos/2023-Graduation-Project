#include "pch.hpp"
#include "Framework.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "PlayingSession.hpp"
#include "Packet.hpp"

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool);
void TimerWorker(std::stop_source& stopper, Framework& me);
void DBaseWorker(std::stop_source& stopper, Framework& me);

Framework::Framework(unsigned int concurrent_hint)
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider()
	, concurrentsNumber(concurrent_hint), concurrentWatcher(concurrent_hint)
	, myWorkers(), workersBreaker()
	, timerWorker(nullptr), timerQueue()
	, databaseWorker(nullptr), databaseQueue()
	, everyRooms(), everySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, login_succeed(), login_failure(), cached_pk_server_info(0, srv::MAX_USERS, srv::GAME_VERSION)
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	workersBreaker.request_stop();

	SleepEx(1000, TRUE);

	Release();
	WSACleanup();

	for (auto& th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
}

void Framework::Awake(unsigned short port_tcp)
{
	std::cout << "������ �غ��ϴ� ��...\n";

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "�ڿ��� �ҷ����� ��...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
	std::cout << "�ڿ��� �ҷ����� �Ϸ�\n";
}

void Framework::Start()
{
	std::cout << "������ �����ϴ� ��...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	auto stopper = std::ref(workersBreaker);
	auto me = std::ref(*this);

	std::cout << "�� �۾� �����带 �õ��ϴ� ��...";
	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, stopper, me, std::ref(myAsyncProvider));
	}
	std::cout << std::ios::right << "�� �۾� �������� ��: " << concurrentsNumber << "��\n";

	std::cout << "Ÿ�̸� �۾� �����带 �õ��ϴ� ��...\n";
	auto& timer_thread = myWorkers.emplace_back(TimerWorker, stopper, me);

	std::cout << "�����ͺ��̽� �����带 �õ��ϴ� ��...\n";
	auto& db_thread = myWorkers.emplace_back(DBaseWorker, stopper, me);

	std::cout << "���� ���۵�!\n";
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			if (concurrentWatcher.try_wait())
			{
				std::cout << "���� ���� ��...\n";

				break;
			}

			SleepEx(10, TRUE);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "���ܷ� ���� ���� ���ͷ�Ʈ: " << e.what() << std::endl;
	}
}

void Framework::Release()
{
	std::cout << "���� ����\n";
}

void Framework::ProceedAsync(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
{
	const auto operation = context->myOperation;

	switch (operation)
	{
		case srv::Operations::ACCEPT:
		{
			ProceedAccept(context);
		}
		break;

		case srv::Operations::SEND:
		{
			ProceedSent(context, key, bytes);
		}
		break;

		case srv::Operations::RECV:
		{
			ProceedRecv(context, key, bytes);
		}
		break;

		case srv::Operations::DISPOSE:
		{
			ProceedDispose(context, key);
		}
		break;

		default:
		{

		}
		break;
	}
}

void Framework::ProceedAccept(srv::Asynchron* context)
{
	const SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]] {
		if (CanAcceptPlayer()) [[likely]] {
			AcceptPlayer(target);
		}
		else
		{
			std::cout << "���� ���� �ʰ��Ͽ� �� �̻� ������ ���� �� �����ϴ�.\n";

			// ��������� ���� ����
			if (FALSE == DisconnectEx(target, nullptr, 0, 0)) [[unlikely]] {
				std::cout << "������ ���� �� ���� ���߿� ���� ���ᰡ �����߽��ϴ�.\n";
			}
			else
			{
				myEntryPoint.Push(target);
			}
		}
	}
}

void Framework::ProceedSent(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "�۽źο��� �߸��� ������ ������! (Ű: " << key << ")\n";
	};

	if (0 < buffer_length)
	{
		const auto left = buffer_length - bytes;
		if (0 < left)
		{
			session->Send(context, static_cast<unsigned>(bytes));
		}
		else if (bytes <= 0)
		{
			std::cout << "�۽� ���� �߻�: ������ ����Ʈ ���� 0��.\n";
			delete context;
		}
		else [[likely]] {
			std::cout << "�۽� �Ϸ�.\n";
			delete context;
		}
	}
	else
	{
		delete context;
	}
}

void Framework::ProceedRecv(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "���źο��� �߸��� ������ ������! (Ű: " << key << ")\n";
	};

	if (0 == bytes) [[unlikely]] // ���� ������ �̹� GetQueueCompletionStatus���� �Ÿ���
	{
		if (!session->isFirst) [[likely]] {
			std::cout << "���� ���� �߻�: ������ ����Ʈ ���� 0��.\n";

			BeginDisconnect(session.get());
		}
		else
		{
			session->Acquire();

			session->SetReceiveVirgin(false); // Page lock ����

			const auto result = session->Recv(BUFSIZ); // �������� ù��° ����
			if (srv::CheckError(result))
			{
				const int error = WSAGetLastError();
				if (!srv::CheckPending(error)) [[unlikely]] {
					std::cout << "ù ���ſ��� ���� �߻�! (ID: " << key << ")\n";
				}
			}

			session->Release();
		}
	}
	else
	{
		// ��Ŷ ó��
		const auto result = session->Swallow(BUFSIZ, bytes);

		if (result)
		{
			session->Acquire();

			const auto& packet = result.value();
			const auto& pk_type = packet->GetProtocol();

			switch (pk_type)
			{
				// �α���
				case srv::Protocol::CS_SIGNIN:
				{
					const auto real_pk = reinterpret_cast<srv::CSPacketSignIn*>(packet);

					const auto user_id = real_pk->userID;
					const auto user_pw = real_pk->userPN;


				}
				break;

				// �α׾ƿ�
				case srv::Protocol::CS_SIGNOUT:
				{
					//const auto real_pk = reinterpret_cast<srv::CSPacketSignUp *>(packet);
				}
				break;

				// ȸ�� ����
				case srv::Protocol::CS_SIGNUP:
				{
					const auto real_pk = reinterpret_cast<srv::CSPacketSignUp*>(packet);
				}
				break;

				// ���� ����
				case srv::Protocol::CS_DISPOSE:
				{
					auto session_state = session->myState.load(std::memory_order_acquire);

					switch (session_state)
					{
						case srv::SessionStates::ROOM_INGAME:
						{
						}
						break;

						case srv::SessionStates::ROOM_COMPLETE:
						{
						}
						break;

						case srv::SessionStates::ROOM_LOBBY:
						{
						}
						break;

						case srv::SessionStates::ROOM_MAIN:
						{
						}
						break;

						case srv::SessionStates::CONNECTED:
						{
							// Ŭ���̾�Ʈ�� ���� ���� (�˸�)
						}
						break;

						case srv::SessionStates::ACCEPTED:
						{
							// Ŭ���̾�Ʈ�� ���� ���� (�˸��� �ʰ� ������)
							session->BeginDisconnect();
							session->Release();
						}
						break;

						case srv::SessionStates::NONE:
						{
							// ����!
						}
						break;
					}

					session->myState.store(session_state, std::memory_order_release);
				}
				break;

				// ����
				case srv::Protocol::CS_REQUEST_VERSION:
				{
					// ��Ŷ ������ �ʿ����.
					auto users_number = numberUsers.load(std::memory_order_acq_rel);

					// ���� �� ����
					cached_pk_server_info.usersCount = users_number;

					// ���� ���� ����
					auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);

					session->BeginSend(asynchron);
					session->Release();
				}
				break;

				// ���� ���
				case srv::Protocol::CS_REQUEST_USERS:
				{
				}
				break;

				// �� ���
				case srv::Protocol::CS_REQUEST_ROOMS:
				{
				}
				break;

				// ��ȭ �޽���
				case srv::Protocol::CS_CHAT:
				{
				}
				break;

				// �� ����
				case srv::Protocol::CS_CREATE_A_ROOM:
				{
				}
				break;

				// �� ����
				case srv::Protocol::CS_LEAVE_A_ROOM:
				{
				}
				break;

				// ���� (���� �������� �� ������)
				case srv::Protocol::CS_DESTROY_A_ROOM:
				{
				}
				break;

				// �� ���� �� ����
				case srv::Protocol::CS_PICK_A_ROOM:
				{
				}
				break;

				// �ڵ� �� ��ġ
				case srv::Protocol::CS_MATCH_A_ROOM:
				{
				}
				break;

				default:
				{
					session->Release();
				}
				break;
			}
		}
	}
}

void Framework::ProceedDispose(srv::Asynchron* context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "������ ���� �߸��� ������ ������! (Ű: " << key << ")\n";
	}
	else
	{
		session->Acquire();
		session->Cleanup();
		session->Release();

		numberUsers--;
	}

	delete context;
}

void Framework::ProceedBeginDiconnect(srv::Asynchron* context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "������ ���� ������ ����! (Ű: " << key << ")\n";
	};

	BeginDisconnect(session);
}

void Worker(std::stop_source& stopper, Framework& me, AsyncPoolService& pool)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED* overlap = nullptr;

	BOOL result{};

	std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";

	constexpr auto aa1 = srv::detail::ratio_leaked<std::ratio<3, 6>>();
	constexpr auto aa2 = srv::detail::ratio_leaked<3, 6>();
	constexpr auto aa3 = srv::detail::ratio_leaked<1.5, 2>();

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested()) [[unlikely]]
		{
			break;
		}

		auto asynchron = static_cast<srv::Asynchron*>(overlap);
		if (TRUE == result) [[likely]]
		{
			me.ProceedAsync(asynchron, key, static_cast<int>(bytes));
		}
		else
		{
			me.ProceedBeginDiconnect(asynchron, key);
		}
	}

	std::cout << "�۾��� ������ " << std::this_thread::get_id() << " ����\n";

	me.concurrentWatcher.arrive_and_wait();
}

void TimerWorker(std::stop_source& stopper, Framework& me)
{
	auto token = stopper.get_token();

	while (true)
	{
		if (token.stop_requested()) [[unlikely]]
		{
			break;
		}
	}

	std::cout << "Ÿ�̸� �۾� ������ " << std::this_thread::get_id() << " ����\n";
}

void DBaseWorker(std::stop_source& stopper, Framework& me)
{
	auto token = stopper.get_token();

	while (true)
	{
		if (token.stop_requested()) [[unlikely]]
		{
			break;
		}
	}

	std::cout << "DB �۾� ������ " << std::this_thread::get_id() << " ����\n";
}

void Framework::BuildSessions()
{
	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto& user : user_sessions)
	{
		user = static_pointer_cast<srv::Session>(make_shared<srv::PlayingSession>(place++));
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto& npc : npc_sessions)
	{
		npc = make_shared<srv::Session>(place++);
	}
}

void Framework::BuildRooms()
{
	for (unsigned i = 0; i < srv::MAX_ROOMS; i++)
	{
		auto& room = everyRooms[i];
		room = make_shared<srv::Room>(i);
	}
}

void Framework::BuildResources()
{}

shared_ptr<srv::Session> Framework::AcceptPlayer(SOCKET target)
{
	auto newbie = SeekNewbiePlace();
	newbie->Acquire();
	newbie->Ready(MakeNewbieID(), target);
	std::cout << "���� �÷��̾� ����: " << target << "\n";

	auto users_number = numberUsers.load(std::memory_order_acquire);

	// ���� �� ����
	cached_pk_server_info.usersCount = users_number;

	// ���� ���� ����
	auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);
	newbie->BeginSend(asynchron);
	newbie->Release();

	numberUsers.store(users_number + 1, std::memory_order_release);
	auto bb = srv::CreatePacket<srv::SCPacketSignInSucceed>(srv::SIGNIN_CAUSE::SUCCEED);

	auto cc = srv::CreateLocalPacket<srv::SCPacketSignInSucceed>(srv::SIGNIN_CAUSE::SUCCEED);

	return newbie;
}

shared_ptr<srv::Session> Framework::ConnectPlayer(unsigned place)
{
	return ConnectPlayer(GetSession(place));
}

shared_ptr<srv::Session> Framework::ConnectPlayer(shared_ptr<srv::Session> session)
{
	session->Acquire();
	std::cout << "�÷��̾� ����: " << session->myID << "\n";

	// �α��� ���� ���� ����
	auto [ticket, asynchron] = srv::CreateTicket(srv::SCPacketSignInSucceed{ srv::SIGNIN_CAUSE::SUCCEED });
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::BeginDisconnect(unsigned place)
{
	BeginDisconnect(GetSession(place).get());
}

void Framework::BeginDisconnect(shared_ptr<srv::Session> session)
{
	BeginDisconnect(session.get());
}

void Framework::BeginDisconnect(srv::Session* session)
{
	session->Acquire();
	session->BeginDisconnect();
	session->Release();

	std::cout << "���� " << session->myPlace << "�� ���� ����. (���� ��" << numberUsers << "��)\n";
}

shared_ptr<srv::Session> Framework::SeekNewbiePlace() const noexcept
{
	auto players_view = everySessions | std::views::take(srv::MAX_USERS);
	auto it = std::find_if(std::execution::par, players_view.begin(), players_view.end()
		, [&](const shared_ptr<srv::Session>& ptr) {
		return ptr->myState == srv::SessionStates::NONE;
	});

	if (players_view.end() == it)
	{
		return nullptr;
	}
	else
	{
		return (*it);
	}
}

unsigned long long Framework::MakeNewbieID() noexcept
{
	return playerIDs++;
}

int Framework::SendTo(srv::Session* session, void* const data, const std::unsigned_integral auto size)
{
	auto asynchron = srv::CreateAsynchron(srv::Operations::SEND);

	auto& wbuffer = asynchron->myBuffer;
	wbuffer.buf = reinterpret_cast<char*>(data);
	wbuffer.len = size;

	return session->BeginSend(asynchron);
}

int Framework::SendServerStatus(srv::Session* session)
{
	auto asynchron = srv::CreateAsynchron(srv::Operations::SEND);

	return 0;
}

int Framework::SendLoginResult(srv::Session* session, login_succeed_t info)
{
	return 0;
}

int Framework::SendLoginResult(srv::Session* session, login_failure_t info)
{
	return 0;
}

bool Framework::CanAcceptPlayer() const noexcept
{
	return numberUsers.load(std::memory_order_acq_rel) < srv::MAX_USERS;
}

bool Framework::CanCreateRoom() const noexcept
{
	return numberRooms.load(std::memory_order_acq_rel) < srv::MAX_ROOMS;
}

shared_ptr<srv::Session> Framework::GetSession(unsigned place) const noexcept(false)
{
	return everySessions.at(place);
}

shared_ptr<srv::Session> Framework::FindSession(unsigned long long id) const noexcept(false)
{
	auto it = std::find_if(std::execution::par_unseq
		, everySessions.cbegin(), everySessions.cend()
		, [id](const shared_ptr<srv::Session>& session) -> bool {
		return (id == session->myID.load(std::memory_order_relaxed));
	});

	if (it != everySessions.cend())
	{
		return *it;
	}
	else
	{
		return nullptr;
	}
}
