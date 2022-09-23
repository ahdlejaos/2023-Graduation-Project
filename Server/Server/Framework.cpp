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
	, myDBProvider()
	, concurrentsNumber(concurrent_hint), concurrentWatcher(concurrent_hint)
	, myWorkers(), workersBreaker()
	, concurrentOutputLock()
	, timerWorker(nullptr), timerQueue()
	, databaseWorker(nullptr), databaseQueue()
	, everyRooms(), everySessions(), lobbySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, login_succeed(), login_failure(), cached_pk_server_info(0, srv::MAX_USERS, srv::GAME_VERSION)
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::ios_base::sync_with_stdio(false);

	std::cin.tie(nullptr);
	std::cerr.tie(nullptr);
	std::clog.tie(nullptr);

	databaseQueue.reserve(100);
}

Framework::~Framework()
{
	Release();

	SleepEx(1000, TRUE);

	WSACleanup();

	for (auto& th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	if (timerWorker->joinable())
	{
		timerWorker->join();
	}

	if (databaseWorker->joinable())
	{
		databaseWorker->join();
	}

	Println("���� ����");
}

void Framework::Awake(unsigned short port_tcp)
{
	std::cout << "������ �غ��ϴ� ��...\n";

	Println("DB ���񽺸� �غ��ϴ� ��...");
	bool db_available = myDBProvider.Awake();
	if (!db_available)
	{
		Println("DB ����!");

		srv::RaiseSystemError(std::errc::not_connected);
	}

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	Println("�ڿ��� �ҷ����� ��...");

	BuildSessions();
	BuildRooms();
	BuildResources();
	Println("�ڿ��� �ҷ����� �Ϸ�");
}

void Framework::Start()
{
	Println("������ �����ϴ� ��...");

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	auto stopper = std::ref(workersBreaker);
	auto me = std::ref(*this);

	Println("�� �۾� �����带 �õ��ϴ� ��...");
	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, stopper, me, std::ref(myAsyncProvider));
	}

	Println(std::right, " (�� �۾� �������� ��: ", concurrentsNumber, "��)");

	Println("Ÿ�̸� �۾� �����带 �õ��ϴ� ��...");
	auto& timer_thread = myWorkers.emplace_back(TimerWorker, stopper, me);

	Println("�����ͺ��̽� �����带 �õ��ϴ� ��...");
	auto& db_thread = myWorkers.emplace_back(DBaseWorker, stopper, me);

	Println("���� ���۵�!");

	DBAddPlayer({ 500, L"yoyofa@naver.com", L"iconer", L"1234!@#$" });
	DBFindPlayer(100);
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			if (concurrentWatcher.try_wait())
			{
				Println("���� ���� ��...");

				break;
			}

			SleepEx(10, TRUE);
		}
	}
	catch (std::exception& e)
	{
		Println("���ܷ� ���� ���� ���ͷ�Ʈ: ", e.what());
	}
}

void Framework::Release()
{
	Println("���� ���� ��...");

	workersBreaker.request_stop();
}

void Framework::DBAddPlayer(UserBlob data)
{
	//auto query = myDBProvider.CreateQuery(L"INSERT INTO Users ([ID], [NICKNAME], [PASSWORD]) VALUES ({}, '{}', '{}')", std::make_wformat_args(data.id, data.nickname, data.password));
	
	//auto query = myDBProvider.CreateQuery(L"INSERT INTO Users ([ID], [NICKNAME], [PASSWORD]) VALUES (300, 'iconer', '12452531')");

	//auto query = myDBProvider.CreateQuery(L"SELECT TOP (1000) * FROM Users");
	
	wchar_t statement[] = L"SELECT TOP (1000) * FROM [Users];";

	SQLHSTMT hstmt{};
	auto sqlcode = SQLAllocHandle(SQL_HANDLE_STMT, myDBProvider.myConnector, &hstmt);

	sqlcode = SQLPrepare(hstmt, (statement), SQL_NTS);
	if (SQLSucceed(sqlcode))
	{
		int result_id{};
		wchar_t result_nickname[32]{};
		wchar_t result_password[32]{};
		SQLLEN result_length{};

		sqlcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &result_id, 0, &result_length);
		
		sqlcode = SQLBindCol(hstmt, 2, SQL_WCHAR, result_nickname, 32, &result_length);
		
		sqlcode = SQLBindCol(hstmt, 3, SQL_WCHAR, result_password, 32, &result_length);

		sqlcode = SQLExecute(hstmt);

		if (SQLStatementHasDiagnotics(sqlcode))
		{
			SQLDiagnostics(SQL_HANDLE_STMT, hstmt);
		}
		else if (SQLSucceed(sqlcode))
		{
			do
			{
				sqlcode = SQLFetch(hstmt);
				
				if (SQLStatementHasDiagnotics(sqlcode))
				{
					SQLDiagnostics(SQL_HANDLE_STMT, hstmt);
				}
			}
			while (!SQLFetchEnded(sqlcode));
		}
	}
	else
	{
		SQLDiagnostics(SQL_HANDLE_STMT, hstmt);
	}

	//query->Bind(1, SQL_INTEGER, &result_id, 1, &id_length);
	//query->Bind(2, &result_nickname, 32, &nn_length);
	//query->Bind(3, &result_password, 32, &pw_length);

	//auto ok = query->Execute();

	SQLSMALLINT result_meta_count = 0;
	//auto status = SQLNumResultCols(query->myQuery, &result_meta_count);

	//status = query->FetchOnce();
}

void Framework::DBFindPlayer(const PID id)
{
	//auto statement = FormatQuery(L"SELECT ID FROM USERS WHERE ID = {}", id);

	wchar_t statement[200]{};

	wsprintf(statement, L"SELECT ID FROM [Users] WHERE ID = 100;");

	SQLHSTMT hstmt{};
	auto sqlcode = SQLAllocHandle(SQL_HANDLE_STMT, myDBProvider.myConnector, &hstmt);

	sqlcode = SQLPrepare(hstmt, (statement), SQL_NTS);
	if (SQLSucceed(sqlcode))
	{
		SQLINTEGER result{};
		SQLLEN result_length{};

		constexpr auto sqltype = sql::Deduct<decltype(result)>();
		
		sqlcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &result, 0, &result_length);

		sqlcode = SQLExecute(hstmt);

		if (SQLStatementHasDiagnotics(sqlcode))
		{
			SQLDiagnostics(SQL_HANDLE_STMT, hstmt);
		}
		else if (SQLSucceed(sqlcode))
		{
		}
	}
	else
	{
		SQLDiagnostics(SQL_HANDLE_STMT, hstmt);
	}

	//auto query = myDBProvider.CreateQuery(statement);

	//int result{};
	//SQLLEN result_length{};
	//query->Bind(1, &result, 1, &result_length);
	//query->Execute();

	//auto status = query->FetchOnce();
}

void Framework::Route(srv::Asynchron* context, ULONG_PTR key, unsigned bytes)
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

	const auto place = static_cast<std::size_t>(key);
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

	const auto place = static_cast<const std::size_t>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "���źο��� �߸��� ������ ������! (Ű: " << key << ")\n";
	};

	if (0 == bytes) [[unlikely]] // ���� ������ �̹� GetQueueCompletionStatus���� �Ÿ���
	{
		if (!session->isFirstCommunication) [[likely]] {
			std::cout << "���� ���� �߻�: ������ ����Ʈ ���� 0��.\n";

			BeginDisconnect(session);
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

					const auto& user_id = real_pk->userAccount;
					const auto& user_pw = real_pk->userPN;

					if (lstrlen(user_id) < 4)
					{
						// �α��� ������ ����!
						login_failure.cause = srv::SIGNIN_CAUSE::FAILURE_WRONG_SINGIN_INFOS;

						SendLoginResult(session.get(), login_failure);
					}
					else
					{
						// �α��� ���� ���� ����
						SendLoginResult(session.get(), login_succeed);
					}
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

	me.Println("�۾��� ������ ", std::this_thread::get_id(), " ����");

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested()) [[unlikely]] {
			break;
		}

		auto asynchron = static_cast<srv::Asynchron*>(overlap);
		if (TRUE == result) [[likely]] {
			me.Route(asynchron, key, static_cast<int>(bytes));
		}
		else
		{
			me.ProceedBeginDiconnect(asynchron, key);
		}
	}

	me.Println("�۾��� ������ ", std::this_thread::get_id(), " ����");

	me.concurrentWatcher.arrive_and_wait();
}

void TimerWorker(std::stop_source& stopper, Framework& me)
{
	auto token = stopper.get_token();

	while (true)
	{
		if (token.stop_requested()) [[unlikely]] {
			break;
		}
	}

	me.Println("Ÿ�̸� �۾� ������ ", std::this_thread::get_id(), " ����");
}

void DBaseWorker(std::stop_source& stopper, Framework& me)
{
	auto token = stopper.get_token();

	while (true)
	{
		if (token.stop_requested()) [[unlikely]] {
			break;
		}
	}

	me.Println("DB �۾� ������ ", std::this_thread::get_id(), " ����");
}

void Framework::BuildSessions()
{
	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto& user : user_sessions)
	{
		user = srv::PlayingSession::Create(place++, myDBProvider);
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto& npc : npc_sessions)
	{
		npc = srv::Session::Create(place++, myDBProvider);
	}
}

void Framework::BuildRooms()
{
	unsigned place = 0;

	for (auto& room : everyRooms)
	{
		room = srv::Room::Create(place++);
	}
}

void Framework::BuildResources()
{}

shared_ptr<srv::Session> Framework::AcceptPlayer(SOCKET target)
{
	auto newbie = SeekNewbiePlace();
	if (newbie)
	{
		newbie->Acquire();
		newbie->Ready(MakeNewbieID(), target);
		std::cout << "���� �÷��̾� ����: " << target << "\n";

		auto users_number = numberUsers.load(std::memory_order_acquire);

		if (users_number < srv::MAX_USERS)
		{
			// ���� �� ����
			cached_pk_server_info.usersCount = users_number + 1;

			// ���� ���� ����
			auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);
			newbie->BeginSend(asynchron);
			newbie->Release();

			numberUsers.store(users_number + 1, std::memory_order_release);
		}
		else
		{

		}
	}
	else
	{

	}

	return newbie;
}

shared_ptr<srv::Session> Framework::ConnectPlayer(const std::size_t place)
{
	return ConnectPlayer(GetSession(place));
}

shared_ptr<srv::Session> Framework::ConnectPlayer(shared_ptr<srv::Session> session)
{
	session->Acquire();

	const auto id = session->GetID();

	std::cout << "�÷��̾� ����: " << id << "\n";

	// �α��� ���� ���� ����
	auto [ticket, asynchron] = srv::CreateTicket(srv::SCPacketSignInSucceed{ id });
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::BeginDisconnect(const std::size_t place)
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

int Framework::SendLoginResult(srv::Session* session, const login_succeed_t& info)
{
	auto [pk, as] = srv::CreateTicket(srv::SCPacketSignInSucceed{ session->myID });

	return session->BeginSend(as);
}

int Framework::SendLoginResult(srv::Session* session, const login_failure_t& info)
{
	auto [pk, as] = srv::CreateTicket(srv::SCPacketSignInFailed{ info.cause });

	return session->BeginSend(as);
}

bool Framework::CanAcceptPlayer() const noexcept
{
	return numberUsers.load(std::memory_order_consume) < srv::MAX_USERS;
}

bool Framework::CanCreateRoom() const noexcept
{
	return numberRooms.load(std::memory_order_consume) < srv::MAX_ROOMS;
}

shared_ptr<srv::Session> Framework::GetSession(const std::size_t place) const noexcept(false)
{
	return everySessions.at(place);
}

shared_ptr<srv::Session> Framework::FindSession(const PID id) const noexcept(false)
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
