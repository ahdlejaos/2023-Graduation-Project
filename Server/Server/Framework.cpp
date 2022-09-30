#include "pch.hpp"
#include "Framework.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "PlayingSession.hpp"
#include "Packet.hpp"

void Worker(std::stop_source& stopper, Framework& me, ConnectService& pool);
void TimerWorker(std::stop_source& stopper, Framework& me);
void DBaseWorker(std::stop_source& stopper, Framework& me);

Framework::Framework(unsigned int concurrent_hint)
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myDatabaseService()
	, concurrentsNumber(concurrent_hint), concurrentWatcher(concurrent_hint)
	, myWorkers(), workersBreaker()
	, concurrentOutputLock()
	, timerWorker(nullptr), timerQueue()
	, databaseWorker(nullptr), databaseAsyncer(srv::Operations::DB_OVERLAPPED)
	, everyRooms(), everySessions(), lobbySessions(), dictSessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, login_succeed(), login_failure(), cached_pk_server_info(0, srv::MAX_USERS, srv::GAME_VERSION)
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::ios_base::sync_with_stdio(false);

	std::cin.tie(nullptr);
	std::cerr.tie(nullptr);
	std::clog.tie(nullptr);
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

	Println("¼­¹ö Á¾·á");
}

void Framework::Awake(unsigned short port_tcp)
{
	std::cout << "¼­¹ö¸¦ ÁØºñÇÏ´Â Áß...\n";

	myEntryPoint.Awake(concurrentsNumber, port_tcp);

	Println("ÀÚ¿øÀ» ºÒ·¯¿À´Â Áß...");
	try
	{
		BuildDatabase();
		BuildSessions();
		BuildRooms();
		BuildResources();
	}
	catch (std::exception& e)
	{
		Println("¸Þ¸ð¸® ÇÒ´ç Áß¿¡ ¿À·ù ¹ß»ý!");
		return;
	}

	Println("ÀÚ¿øÀÇ ºÒ·¯¿À±â ¿Ï·á");
}

void Framework::Start()
{
	Println("¼­¹ö¸¦ ½ÃÀÛÇÏ´Â Áß...");

	myEntryPoint.Start(myID);

	auto stopper = std::ref(workersBreaker);
	auto me = std::ref(*this);

	Println("ÁÖ ÀÛ¾÷ ½º·¹µå¸¦ ½Ãµ¿ÇÏ´Â Áß...");
	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, stopper, me, std::ref(myEntryPoint));
	}

	Println(std::right, " (ÁÖ ÀÛ¾÷ ½º·¹µåÀÇ ¼ö: ", concurrentsNumber, "°³)");

	Println("Å¸ÀÌ¸Ó ÀÛ¾÷ ½º·¹µå¸¦ ½Ãµ¿ÇÏ´Â Áß...");
	auto& timer_thread = myWorkers.emplace_back(TimerWorker, stopper, me);

	Println("µ¥ÀÌÅÍº£ÀÌ½º ½º·¹µå¸¦ ½Ãµ¿ÇÏ´Â Áß...");
	auto& db_thread = myWorkers.emplace_back(DBaseWorker, stopper, me);

	Println("¼­¹ö ½ÃÀÛµÊ!");
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			if (concurrentWatcher.try_wait())
			{
				Println("¼­¹ö Á¾·á Áß...");

				break;
			}

			SleepEx(10, TRUE);
		}
	}
	catch (std::exception& e)
	{
		Println("¿¹¿Ü·Î ÀÎÇÑ ¼­¹ö ÀÎÅÍ·´Æ®: ", e.what());
	}
}

void Framework::Release()
{
	Println("¼­¹ö Á¾·á Áß...");

	workersBreaker.request_stop();
}

void Framework::RouteSucceed(LPWSAOVERLAPPED context, ULONG_PTR key, unsigned bytes)
{
	auto asynchron = static_cast<srv::BasicContext*>(context);
	const auto operation = asynchron->myOperation;

	switch (operation)
	{
		case srv::Operations::ACCEPT:
		{
			ProceedAccept(asynchron);
		}
		break;

		case srv::Operations::SEND:
		{
			ProceedSent(asynchron, key, bytes);
		}
		break;

		case srv::Operations::RECV:
		{
			ProceedRecv(asynchron, key, bytes);
		}
		break;

		case srv::Operations::DISPOSE:
		{
			ProceedDispose(asynchron, key);
		}
		break;

		default:
		{

		}
		break;
	}
}

void Framework::RouteFailed(LPWSAOVERLAPPED context, ULONG_PTR key, unsigned bytes)
{
	auto asynchron = static_cast<srv::Asynchron*>(context);
	const auto operation = asynchron->myOperation;

	switch (operation)
	{
		case srv::Operations::ACCEPT:
		{
			ProceedBeginDiconnect(key);
		}
		break;

		case srv::Operations::SEND:
		{
			std::cout << "¼ö½Å ÀÛ¾÷ ½ÇÆÐ: " << key << "\n";
			if (0 == bytes)
			{
				std::cout << "¼ö½Å ¿À·ù ¹ß»ý: º¸³»´Â ¹ÙÀÌÆ® ¼ö°¡ 0ÀÓ.\n";
			}
		}
		break;

		case srv::Operations::RECV:
		{
			std::cout << "¼Û½Å ÀÛ¾÷ ½ÇÆÐ: " << key << "\n";
			if (0 == bytes)
			{
				std::cout << "¼Û½Å ¿À·ù ¹ß»ý: ¹Þ´Â ¹ÙÀÌÆ® ¼ö°¡ 0ÀÓ.\n";
			}

			ProceedBeginDiconnect(key);
		}
		break;

		case srv::Operations::DISPOSE:
		{
			std::cout << "¿¬°á ²÷±â ÀÛ¾÷ ½ÇÆÐ: " << key << "\n";
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
			std::cout << "À¯Àú ¼ö°¡ ÃÊ°úÇÏ¿© ´õ ÀÌ»ó Á¢¼ÓÀ» ¹ÞÀ» ¼ö ¾ø½À´Ï´Ù.\n";

			// µ¿±â½ÄÀ¸·Î Á¢¼Ó Á¾·á
			if (FALSE == DisconnectEx(target, nullptr, 0, 0)) [[unlikely]] {
				std::cout << "¿¬°áÀ» ¹ÞÀ» ¼ö ¾ø´Â ¿ÍÁß¿¡ Á¢¼Ó Á¾·á°¡ ½ÇÆÐÇß½À´Ï´Ù.\n";
			}
			else
			{
				myEntryPoint.Push(target);
			}
		}
	}
}

void Framework::ProceedSent(srv::BasicContext* context, ULONG_PTR key, unsigned bytes)
{
	auto& wbuffer = context->myBuffer;
	auto& buffer = wbuffer.buf;
	auto& buffer_length = wbuffer.len;

	const auto place = static_cast<std::size_t>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "¼Û½ÅºÎ¿¡¼­ Àß¸øµÈ ¼¼¼ÇÀ» ÂüÁ¶ÇÔ! (Å°: " << key << ")\n";
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
			std::cout << "¼Û½Å ¿À·ù ¹ß»ý: º¸³»´Â ¹ÙÀÌÆ® ¼ö°¡ 0ÀÓ.\n";
			delete context;
		}
		else [[likely]] {
			std::cout << "¼Û½Å ¿Ï·á.\n";
			delete context;
		}
	}
	else
	{
		delete context;
	}
}

void Framework::ProceedRecv(srv::BasicContext* context, ULONG_PTR key, unsigned bytes)
{
	const auto place = static_cast<const std::size_t>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "¼ö½ÅºÎ¿¡¼­ Àß¸øµÈ ¼¼¼ÇÀ» ÂüÁ¶ÇÔ! (Å°: " << key << ")\n";
		return;
	};

	// ÆÐÅ¶ Ã³¸®
	const auto result = session->Swallow(BUFSIZ, bytes);

	if (result)
	{
		session->Acquire();

		const auto& packet = result.value();
		const auto& pk_type = packet->GetProtocol();

		switch (pk_type)
		{
			// ·Î±×ÀÎ
			case srv::Protocol::CS_SIGNIN:
			{
				const auto real_pk = reinterpret_cast<srv::CSPacketSignIn*>(packet);

				const auto& user_id = real_pk->userAccount;
				const auto& user_pw = real_pk->userPN;

				if (lstrlen(user_id) < 4)
				{
					// ·Î±×ÀÎ ¹«Á¶°Ç ½ÇÆÐ!
					login_failure.cause = srv::SIGNIN_CAUSE::FAILURE_WRONG_SINGIN_INFOS;

					SendLoginResult(session.get(), login_failure);
				}
				else
				{
					auto post = PostDatabaseJob(key, 0);


					BasicUserBlob blob{
						.id = 10,
					};

					// query_check_user: ·Î±×ÀÎ ¼º°ø ¿©ºÎ °ËÁõ
					auto& query = DBFindPlayer(user_id);

					static SQLINTEGER result_id{};
					static SQLWCHAR result_nickname[30]{};
					static SQLLEN result_length{};

					query.Bind(1, &result_id, 0, &result_length);
					query.Bind(2, result_nickname, 30, &result_length);

					SendLoginResult(session.get(), login_succeed);
				}
			}
			break;

			// ·Î±×¾Æ¿ô
			case srv::Protocol::CS_SIGNOUT:
			{
				//const auto real_pk = reinterpret_cast<srv::CSPacketSignUp *>(packet);
			}
			break;

			// È¸¿ø °¡ÀÔ
			case srv::Protocol::CS_SIGNUP:
			{
				const auto real_pk = reinterpret_cast<srv::CSPacketSignUp*>(packet);
			}
			break;

			// ¹ü¿ë Á¾·á
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
						// Å¬¶óÀÌ¾ðÆ®ÀÇ Á¢¼Ó Á¾·á (¾Ë¸²)
					}
					break;

					case srv::SessionStates::ACCEPTED:
					{
						// Å¬¶óÀÌ¾ðÆ®ÀÇ Á¢¼Ó Á¾·á (¾Ë¸®Áö ¾Ê°í Á¶¿ëÈ÷)
						session->Disconnect();
						myEntryPoint.Push(session->mySocket);
						session->Release();
					}
					break;

					case srv::SessionStates::NONE:
					{
						// ¿À·ù!
					}
					break;
				}

				session->myState.store(session_state, std::memory_order_release);
			}
			break;

			// ¹öÀü
			case srv::Protocol::CS_REQUEST_VERSION:
			{
				// ÆÐÅ¶ Á¤º¸´Â ÇÊ¿ä¾ø´Ù.
				auto users_number = numberUsers.load(std::memory_order_acq_rel);

				// À¯Àú ¼ö °»½Å
				cached_pk_server_info.usersCount = users_number;

				// ¼­¹ö »óÅÂ Àü¼Û
				auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);

				session->BeginSend(asynchron);
				session->Release();
			}
			break;

			// À¯Àú ¸ñ·Ï
			case srv::Protocol::CS_REQUEST_USERS:
			{
			}
			break;

			// ¹æ ¸ñ·Ï
			case srv::Protocol::CS_REQUEST_ROOMS:
			{
			}
			break;

			// ´ëÈ­ ¸Þ½ÃÁö
			case srv::Protocol::CS_CHAT:
			{
			}
			break;

			// ¹æ »ý¼º
			case srv::Protocol::CS_CREATE_A_ROOM:
			{
			}
			break;

			// ¹æ ³ª°¨
			case srv::Protocol::CS_LEAVE_A_ROOM:
			{
			}
			break;

			// ¹æÆø (¹æÀÇ À¯ÀúµéÀº ¹æ ³ª°¡±â)
			case srv::Protocol::CS_DESTROY_A_ROOM:
			{
			}
			break;

			// ¹æ ¼±ÅÃ ÈÄ ÀÔÀå
			case srv::Protocol::CS_PICK_A_ROOM:
			{
			}
			break;

			// ÀÚµ¿ ¹æ ¸ÅÄ¡
			case srv::Protocol::CS_MATCH_A_ROOM:
			{
			}
			break;
		}

		//session->Release();
	}
}

void Framework::ProceedDispose(srv::BasicContext* context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "¿¬°áÀ» ²÷À» Àß¸øµÈ ¼¼¼ÇÀ» ÂüÁ¶ÇÔ! (Å°: " << key << ")\n";
	}
	else
	{
		auto rit = dictSessions.find(session->GetID());
		dictSessions.erase(rit);

		myEntryPoint.Push(session->mySocket);

		session->Acquire();
		session->Cleanup();
		session->Release();

		numberUsers--;
	}
}

void Framework::ProceedBeginDiconnect(ULONG_PTR key)
{
	const auto place = static_cast<size_t>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "¿¬°áÀ» ²÷À» ¼¼¼ÇÀÌ ¾øÀ½! (Å°: " << key << ")\n";
		return;
	};

	BeginDisconnect(session);
}

void Framework::ProceedBeginDiconnect(shared_ptr<srv::Session> session)
{
	BeginDisconnect(session);
}

void Worker(std::stop_source& stopper, Framework& me, ConnectService& svc)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED* overlap = nullptr;

	BOOL result{};

	me.Println("ÀÛ¾÷ÀÚ ½º·¹µå ", std::this_thread::get_id(), " ½ÃÀÛ");

	while (true)
	{
		result = svc.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested()) [[unlikely]] {
			break;
		}

			if (TRUE == result) [[likely]] {
				me.RouteSucceed(overlap, key, static_cast<int>(bytes));
			}
			else
			{
				me.RouteFailed(overlap, key, static_cast<int>(bytes));
			}
	}

	me.Println("ÀÛ¾÷ÀÚ ½º·¹µå ", std::this_thread::get_id(), " Á¾·á");

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

	me.Println("Å¸ÀÌ¸Ó ÀÛ¾÷ ½º·¹µå ", std::this_thread::get_id(), " Á¾·á");
}

void DBaseWorker(std::stop_source& stopper, Framework& me)
{
	auto token = stopper.get_token();

	auto& service = me.myDatabaseService;
	while (true)
	{
		if (token.stop_requested()) [[unlikely]] {
			break;
		};

		// 
		if (!service.IsEmpty())
		{
			auto job = service.PopJob();

			auto sqlcode = job->Execute();
			sqlcode = job->Fetch();
		}

		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(0.01s);
	}

	me.Println("DB ÀÛ¾÷ ½º·¹µå ", std::this_thread::get_id(), " Á¾·á");
}

shared_ptr<srv::Session> Framework::AcceptPlayer(SOCKET target)
{
	auto newbie = SeekNewbiePlace();
	if (newbie)
	{
		newbie->Acquire();
		newbie->Ready(MakeNewbieID(), target);
		std::cout << "¿¹ºñ ÇÃ·¹ÀÌ¾î Á¢¼Ó: " << target << "\n";

		auto users_number = numberUsers.load(std::memory_order_acquire);

		if (users_number < srv::MAX_USERS)
		{
			// À¯Àú ¼ö °»½Å
			cached_pk_server_info.usersCount = users_number + 1;

			// ¼­¹ö »óÅÂ Àü¼Û
			auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);
			newbie->BeginSend(asynchron);
			newbie->Release();

			numberUsers.store(users_number + 1, std::memory_order_release);

			newbie->BeginRecv();
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

	std::cout << "ÇÃ·¹ÀÌ¾î Á¢¼Ó: " << id << "\n";

	// ·Î±×ÀÎ ¼º°ø ¿©ºÎ Àü¼Û
	auto [ticket, asynchron] = srv::CreateTicket(srv::SCPacketSignInSucceed{ id });
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::ProceedBeginDiconnect(ULONG_PTR key)
{
	const auto place = static_cast<size_t>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]] {
		std::cout << "ì—°ê²°ì„ ëŠì„ ì„¸ì…˜ì´ ì—†ìŒ! (í‚¤: " << key << ")\n";
		return;
	};

	BeginDisconnect(session);
}

void Framework::ProceedBeginDiconnect(shared_ptr<srv::Session> session)
{
	BeginDisconnect(session);
}

void Framework::ProceedBeginDiconnect(srv::Session* session)
{
	BeginDisconnect(session);
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

	std::cout << "¼¼¼Ç " << session->myPlace << "ÀÇ ¿¬°á ²÷±è. (À¯Àú ¼ö: " << numberUsers << "¸í)\n";
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

BOOL Framework::PostDatabaseJob(const PID user_id, const DWORD data)
{
	return myEntryPoint.Post(data, static_cast<ULONG_PTR>(user_id), &databaseAsyncer);
}

BOOL Framework::PostDatabaseJob(const PID user_id, const srv::DatabaseTasks type, void* blob)
{
	return 0;
}

db::Query& Framework::DBAddPlayer(BasicUserBlob data)
{
	return myDatabaseService.PushJob(std::vformat(L"INSERT INTO [Users] (ID, NICKNAME, PASSWORD) VALUES ({}, '{}', '{}');", std::make_wformat_args(data.id, data.nickname, data.password)));

	//static SQLINTEGER result{};
	//static SQLLEN result_length{};

	//query.Bind(1, &result, 0, &result_length);
	//auto ok = query.Execute();
	//auto sqlcode = query.Fetch();
}

db::Query& Framework::DBFindPlayer(const std::wstring_view& email)
{
	return myDatabaseService.PushJob
	(
		std::vformat
		(
			L"SELECT [bf_user].[ID], [bf_user].[NICKNAME]"
			"FROM [Users] AS [bf_user], [UserStaticInfos] AS [bf_info]"
			"WHERE [bf_info].[EMAIL] = 'iconer'"

			, std::make_wformat_args(email)
		));
}

db::Query& Framework::DBFindPlayerByNickname(const std::wstring_view& nickname)
{
	return myDatabaseService.PushJob
	(
		std::vformat
		(L""

			, std::make_wformat_args(nickname)
		));

		//return myDatabaseService.PushJob(std::vformat(L"SELECT [ID], [NICKNAME] FROM [Users] WHERE [ID] = {};", std::make_wformat_args(100)));
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

void Framework::BuildDatabase()
{
	Println("DB ¼­ºñ½º¸¦ ÁØºñÇÏ´Â Áß...");

	if (!myDatabaseService.Awake())
	{
		Println("µ¥ÀÌÅÍº£ÀÌ½º ¿À·ù!");

		srv::RaiseSystemError(std::errc::operation_not_permitted);
	}

	myDatabaseService.Start();
}

void Framework::BuildSessions()
{
	dictSessions.reserve(srv::MAX_USERS);

	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto& user : user_sessions)
	{
		user = srv::PlayingSession::Create(place++, myDatabaseService);
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto& npc : npc_sessions)
	{
		npc = srv::Session::Create(place++, myDatabaseService);
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
