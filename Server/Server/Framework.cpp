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
	std::cout << "서버를 준비하는 중...\n";

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "자원을 불러오는 중...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
	std::cout << "자원의 불러오기 완료\n";
}

void Framework::Start()
{
	std::cout << "서버를 시작하는 중...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	auto stopper = std::ref(workersBreaker);
	auto me = std::ref(*this);

	std::cout << "주 작업 스레드를 시동하는 중...";
	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto& th = myWorkers.emplace_back(Worker, stopper, me, std::ref(myAsyncProvider));
	}
	std::cout << std::ios::right << "주 작업 스레드의 수: " << concurrentsNumber << "개\n";

	std::cout << "타이머 작업 스레드를 시동하는 중...\n";
	auto& timer_thread = myWorkers.emplace_back(TimerWorker, stopper, me);

	std::cout << "데이터베이스 스레드를 시동하는 중...\n";
	auto& db_thread = myWorkers.emplace_back(DBaseWorker, stopper, me);

	std::cout << "서버 시작됨!\n";
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			if (concurrentWatcher.try_wait())
			{
				std::cout << "서버 종료 중...\n";

				break;
			}

			SleepEx(10, TRUE);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "예외로 인한 서버 인터럽트: " << e.what() << std::endl;
	}
}

void Framework::Release()
{
	std::cout << "서버 종료\n";
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
			std::cout << "유저 수가 초과하여 더 이상 접속을 받을 수 없습니다.\n";

			// 동기식으로 접속 종료
			if (FALSE == DisconnectEx(target, nullptr, 0, 0)) [[unlikely]] {
				std::cout << "연결을 받을 수 없는 와중에 접속 종료가 실패했습니다.\n";
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
		std::cout << "송신부에서 잘못된 세션을 참조함! (키: " << key << ")\n";
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
			std::cout << "송신 오류 발생: 보내는 바이트 수가 0임.\n";
			delete context;
		}
		else [[likely]] {
			std::cout << "송신 완료.\n";
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
		std::cout << "수신부에서 잘못된 세션을 참조함! (키: " << key << ")\n";
	};

	if (0 == bytes) [[unlikely]] // 연결 끊김은 이미 GetQueueCompletionStatus에서 거른다
	{
		if (!session->isFirst) [[likely]] {
			std::cout << "수신 오류 발생: 보내는 바이트 수가 0임.\n";

			BeginDisconnect(session.get());
		}
		else
		{
			session->Acquire();

			session->SetReceiveVirgin(false); // Page lock 해제

			const auto result = session->Recv(BUFSIZ); // 실질적인 첫번째 수신
			if (srv::CheckError(result))
			{
				const int error = WSAGetLastError();
				if (!srv::CheckPending(error)) [[unlikely]] {
					std::cout << "첫 수신에서 오류 발생! (ID: " << key << ")\n";
				}
			}

			session->Release();
		}
	}
	else
	{
		// 패킷 처리
		const auto result = session->Swallow(BUFSIZ, bytes);

		if (result)
		{
			session->Acquire();

			const auto& packet = result.value();
			const auto& pk_type = packet->GetProtocol();

			switch (pk_type)
			{
				// 로그인
				case srv::Protocol::CS_SIGNIN:
				{
					const auto real_pk = reinterpret_cast<srv::CSPacketSignIn*>(packet);

					const auto user_id = real_pk->userID;
					const auto user_pw = real_pk->userPN;


				}
				break;

				// 로그아웃
				case srv::Protocol::CS_SIGNOUT:
				{
					//const auto real_pk = reinterpret_cast<srv::CSPacketSignUp *>(packet);
				}
				break;

				// 회원 가입
				case srv::Protocol::CS_SIGNUP:
				{
					const auto real_pk = reinterpret_cast<srv::CSPacketSignUp*>(packet);
				}
				break;

				// 범용 종료
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
							// 클라이언트의 접속 종료 (알림)
						}
						break;

						case srv::SessionStates::ACCEPTED:
						{
							// 클라이언트의 접속 종료 (알리지 않고 조용히)
							session->BeginDisconnect();
							session->Release();
						}
						break;

						case srv::SessionStates::NONE:
						{
							// 오류!
						}
						break;
					}

					session->myState.store(session_state, std::memory_order_release);
				}
				break;

				// 버전
				case srv::Protocol::CS_REQUEST_VERSION:
				{
					// 패킷 정보는 필요없다.
					auto users_number = numberUsers.load(std::memory_order_acq_rel);

					// 유저 수 갱신
					cached_pk_server_info.usersCount = users_number;

					// 서버 상태 전송
					auto [ticket, asynchron] = srv::CreateTicket(cached_pk_server_info);

					session->BeginSend(asynchron);
					session->Release();
				}
				break;

				// 유저 목록
				case srv::Protocol::CS_REQUEST_USERS:
				{
				}
				break;

				// 방 목록
				case srv::Protocol::CS_REQUEST_ROOMS:
				{
				}
				break;

				// 대화 메시지
				case srv::Protocol::CS_CHAT:
				{
				}
				break;

				// 방 생성
				case srv::Protocol::CS_CREATE_A_ROOM:
				{
				}
				break;

				// 방 나감
				case srv::Protocol::CS_LEAVE_A_ROOM:
				{
				}
				break;

				// 방폭 (방의 유저들은 방 나가기)
				case srv::Protocol::CS_DESTROY_A_ROOM:
				{
				}
				break;

				// 방 선택 후 입장
				case srv::Protocol::CS_PICK_A_ROOM:
				{
				}
				break;

				// 자동 방 매치
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
		std::cout << "연결을 끊을 잘못된 세션을 참조함! (키: " << key << ")\n";
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
		std::cout << "연결을 끊을 세션이 없음! (키: " << key << ")\n";
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

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 시작\n";

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

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";

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

	std::cout << "타이머 작업 스레드 " << std::this_thread::get_id() << " 종료\n";
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

	std::cout << "DB 작업 스레드 " << std::this_thread::get_id() << " 종료\n";
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
	std::cout << "예비 플레이어 접속: " << target << "\n";

	auto users_number = numberUsers.load(std::memory_order_acquire);

	// 유저 수 갱신
	cached_pk_server_info.usersCount = users_number;

	// 서버 상태 전송
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
	std::cout << "플레이어 접속: " << session->myID << "\n";

	// 로그인 성공 여부 전송
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

	std::cout << "세션 " << session->myPlace << "의 연결 끊김. (유저 수" << numberUsers << "명)\n";
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
