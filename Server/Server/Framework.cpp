#include "pch.hpp"
#include "Framework.hpp"
#include "Asynchron.hpp"
#include "Room.hpp"
#include "Session.hpp"
#include "PlayingSession.hpp"

void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool);

Framework::Framework()
	: myID(srv::SERVER_ID)
	, myEntryPoint(), myAsyncProvider(), concurrentsNumber(0), myWorkers()
	, everyRooms(), everySessions()
	, numberRooms(0), numberUsers(0)
	, lastPacketType(srv::Protocol::NONE)
	, myPipelineBreaker()
	, login_succeed(), login_failure()
{
	//std::cout.imbue(std::locale{ "KOREAN" });
	std::cout.sync_with_stdio(false);
}

Framework::~Framework()
{
	WSACleanup();

	for (auto &th : myWorkers)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
}

void Framework::Awake(unsigned int concurrent_hint, unsigned short port_tcp)
{
	std::cout << "서버 시동 중...\n";

	concurrentsNumber = concurrent_hint;

	myAsyncProvider.Awake(concurrentsNumber);
	myEntryPoint.Awake(port_tcp);

	std::cout << "자원 생성 중...\n";
	BuildSessions();
	BuildRooms();
	BuildResources();
}

void Framework::Start()
{
	std::cout << "서버 시작하는 중...\n";

	myAsyncProvider.Link(myEntryPoint.serverSocket, myID);
	myEntryPoint.Start();

	for (unsigned i = 0; i < concurrentsNumber; i++)
	{
		auto &th = myWorkers.emplace_back(Worker, std::ref(myPipelineBreaker), std::ref(*this), std::ref(myAsyncProvider));
	}

	std::cout << "서버 시작\n";
}

void Framework::Update()
{
	try
	{
		while (true)
		{
			SleepEx(10, TRUE);
		}
	}
	catch (std::exception &e)
	{
		std::cout << "예외로 인한 서버 인터럽트: " << e.what() << std::endl;
	}

	std::cout << "서버 종료 중...\n";
}

void Framework::Release()
{
	myPipelineBreaker.request_stop();

	std::cout << "서버 종료\n";
}

void Framework::ProceedAsync(Asynchron *context, ULONG_PTR key, unsigned bytes)
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

void Framework::ProceedAccept(Asynchron *context)
{
	const SOCKET target = myEntryPoint.Update();

	if (NULL != target) [[likely]]
	{
		if (numberUsers < srv::MAX_USERS) [[likely]]
		{
			AcceptPlayer(target);
			numberUsers++;
		}
		else
		{
			std::cout << "유저 수가 초과하여 더 이상 접속을 받을 수 없습니다.\n";

			if (FALSE == DisconnectEx(target, srv::CreateAsynchron(srv::Operations::DISPOSE), 0, 0)) [[unlikely]]
			{
				std::cout << "비동기 연결 해제가 실패했습니다.\n";
			}
		}
	}
}

void Framework::ProceedDiconnect(Asynchron *context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "연결을 끊을 세션이 없음! (키: " << key << ")\n";
	};

	session->Disconnect();
}

void Framework::ProceedSent(Asynchron *context, ULONG_PTR key, unsigned bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
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
		else [[likely]]
		{
			std::cout << "송신 완료.\n";
			delete context;
		}
	}
	else
	{
		delete context;
	}
}

void Framework::ProceedRecv(Asynchron *context, ULONG_PTR key, unsigned bytes)
{
	auto &wbuffer = context->myBuffer;
	auto &buffer = wbuffer.buf;
	auto &buffer_length = wbuffer.len;

	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "수신부에서 잘못된 세션을 참조함! (키: " << key << ")\n";
	};

	if (0 == bytes) [[unlikely]] // 연결 끊김은 이미 GetQueueCompletionStatus에서 거른다
	{
		if (!context->isFirst) [[likely]]
		{
			std::cout << "수신 오류 발생: 보내는 바이트 수가 0임.\n";

			Disconnect(session.get());
		}
		else
		{
			context->isFirst = false; // Page lock 해제

			session->Recv(BUFSIZ); // 실질적인 첫번째 수신
		}
	}
	else
	{
		// 패킷 처리
		session->Swallow(BUFSIZ, bytes);
	}
}

void Framework::ProceedDispose(Asynchron *context, ULONG_PTR key)
{
	const auto place = static_cast<unsigned>(key);
	auto session = GetSession(place);
	if (!session) [[unlikely]]
	{
		std::cout << "연결을 끊을 잘못된 세션을 참조함! (키: " << key << ")\n";
	}
	else
	{
		session->Acquire();
		session->Dispose();
		session->Release();

		numberUsers--;
	}

	delete context;
}

void Worker(std::stop_source &stopper, Framework &me, AsyncPoolService &pool)
{
	auto token = stopper.get_token();
	DWORD bytes = 0;
	ULONG_PTR key = 0;
	WSAOVERLAPPED *overlap = nullptr;

	BOOL result{};

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 시작\n";

	while (true)
	{
		result = pool.Async(std::addressof(bytes), std::addressof(key), std::addressof(overlap));

		if (token.stop_requested()) [[unlikely]]
		{
			std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
			break;
		}

		auto asynchron = static_cast<Asynchron *>(overlap);
		if (TRUE == result) [[likely]]
		{
			me.ProceedAsync(asynchron, key, static_cast<int>(bytes));
		}
		else
		{
			me.ProceedDiconnect(asynchron, key);
		}
	}

	std::cout << "작업자 스레드 " << std::this_thread::get_id() << " 종료\n";
}

void Framework::BuildSessions()
{
	auto user_sessions = everySessions | std::views::take(srv::MAX_USERS);

	unsigned place = srv::USERS_ID_BEGIN;
	for (auto &user : user_sessions)
	{
		user = static_pointer_cast<Session>(make_shared<PlayingSession>(place++));
	}

	auto npc_sessions = everySessions | std::views::drop(srv::MAX_USERS);

	place = srv::NPC_ID_BEGIN;
	for (auto &npc : npc_sessions)
	{
		npc = make_shared<Session>(place++);
	}
}

void Framework::BuildRooms()
{
	for (unsigned i = 0; i < srv::MAX_ROOMS; i++)
	{
		auto &room = everyRooms[i];
		room = make_shared<Room>(i);
	}
}

void Framework::BuildResources()
{}

shared_ptr<Session> Framework::AcceptPlayer(SOCKET target)
{
	auto newbie = SeekNewbiePlace();
	newbie->Acquire();
	newbie->Ready(MakeNewbieID(), target);
	std::cout << "예비 플레이어 접속: " << target << "\n";

	// 서버 상태 전송
	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketServerInfo>(numberUsers.load(std::memory_order_relaxed), srv::MAX_USERS, srv::GAME_VERSION);
	newbie->BeginSend(asynchron);
	newbie->Release();

	return newbie;
}

shared_ptr<Session> Framework::ConnectPlayer(unsigned place)
{
	return ConnectPlayer(GetSession(place));
}

shared_ptr<Session> Framework::ConnectPlayer(shared_ptr<Session> session)
{
	session->Acquire();
	std::cout << "플레이어 접속: " << session->myID << "\n";

	// 로그인 성공 여부 전송
	auto [ticket, asynchron] = srv::CreateTicket<srv::SCPacketSignUp>();
	session->BeginSend(asynchron);
	session->Connect();

	session->Release();

	return session;
}

void Framework::Disconnect(unsigned place)
{
	Disconnect(GetSession(place).get());
}

void Framework::Disconnect(shared_ptr<Session> session)
{
	Disconnect(session.get());

}

void Framework::Disconnect(Session *session)
{
	session->Acquire();
	session->Disconnect();
	session->Release();

	std::cout << "세션 " << session->myPlace << "의 연결 끊김. (유저 수" << numberUsers << "명)\n";
}

shared_ptr<Session> Framework::SeekNewbiePlace() const noexcept
{
	auto players_view = everySessions | std::views::take(srv::MAX_USERS);
	auto it = std::find_if(std::execution::par, players_view.begin(), players_view.end()
		, [&](const shared_ptr<Session> &ptr) {
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

template<std::unsigned_integral Integral>
int Framework::SendTo(Session *session, void *const data, const Integral size)
{
	auto asynchron = srv::CreateAsynchron(srv::Operations::SEND);

	auto &wbuffer = asynchron->myBuffer;
	wbuffer.buf = reinterpret_cast<char*>(data);
	wbuffer.len = size;

	return session->BeginSend(asynchron);
}

int Framework::SendServerStatus(Session *session)
{
	return 0;
}

int Framework::SendLoginResult(Session *session, login_succeed_t info)
{
	return 0;
}

int Framework::SendLoginResult(Session *session, login_failure_t info)
{
	return 0;
}

shared_ptr<Session> Framework::GetSession(unsigned place) const noexcept(false)
{
	return everySessions.at(place);
}
