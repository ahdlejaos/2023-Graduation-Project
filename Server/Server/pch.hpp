#pragma once

#ifndef  __PCH__
#define __PCH__
#include "stdafx.hpp"

class Framework;
class TimedJob;
class DatabaseJob;

class ConnectService;
class AsyncPoolService;
class DatabaseService;
class DatabaseQuery;
template<typename Ty> class BasicDatabaseJob;
class DatabaseJob;

struct BasicUserBlob;
struct UserBlob;
struct TimerBlob;

class GameObject;
class GameEntity;
class GameTerrain;

class Player;

using PID = unsigned long long;

namespace srv
{
	class Context;
	class Asynchron;
	class DatabaseContext;
	class Session;
	class PlayingSession;
	class Room;

	constexpr wchar_t GAME_VERSION[] = L"0.0.1";

	// 서버 스레드 수
	constexpr unsigned int THREADS_COUNT = 6;
	// 서버 TCP 포트
	constexpr unsigned short SERVER_PORT_TCP = 12000;
	// 서버 UDP 포트
	constexpr unsigned short SERVER_PORT_UDP = 12001;
	// 서버 식별자
	constexpr ULONG_PTR SERVER_ID = ULONG_PTR(-1);

	namespace detail
	{
		constexpr unsigned FRAME = 30;

		using Framerate = std::ratio<FRAME, 1>;
		using Tick = std::ratio_divide<std::ratio<1>, Framerate>;

		template <intmax_t N = 0, intmax_t D = 1, typename Ty = void>
		struct ratio_2s_fn;

		template <intmax_t N, intmax_t D>
		struct ratio_2s_fn<N, D, void>
		{
			static let double value = static_cast<double>(N) / static_cast<double>(D);

			let double operator()(const std::ratio<N, D>&) const noexcept
			{
				return value;
			}

			let double operator()() const noexcept
			{
				return value;
			}
		};

		template <intmax_t N, intmax_t D>
		struct ratio_2s_fn<N, D, std::ratio<N, D>>
		{
			using Ratio = std::ratio<N, D>;

			static let double value = static_cast<double>(Ratio::num) / static_cast<double>(Ratio::den);

			let double operator()(const Ratio&) const noexcept
			{
				return value;
			}

			let double operator()() const noexcept
			{
				return value;
			}
		};

		template <intmax_t N, intmax_t D>
		using Ratio = std::ratio<N, D>;

		template <intmax_t N, intmax_t D>
		ratio_2s_fn(Ratio<N, D>)->ratio_2s_fn<N, D, Ratio<N, D>>;

		template <typename Ty, intmax_t N, intmax_t D>
		concept Rational = std::is_same_v<Ratio<N, D>, Ty> || std::derived_from<Ty, Ratio<N, D>>;

		inline constexpr double ratio_leaked(const Arithmetic auto N, const Arithmetic auto D) noexcept
		{
			return static_cast<double>(N) / static_cast<double>(D);
		}

		template <intmax_t N, intmax_t D, Rational<N, D> Rt>
		inline constexpr double ratio_leaked(const Rt& ratio) noexcept
		{
			return static_cast<double>(N) / static_cast<double>(D);
		}

		template <typename Rt>
			requires
			requires {
			std::same_as<std::ratio_divide<Rt, Rt>, std::ratio<1>>;
			Rt::num;
			Rt::den;
		}
		inline constexpr double ratio_leaked() noexcept
		{
			return static_cast<double>(Rt::num) / static_cast<double>(Rt::den);
		}

		template <Arithmetic auto N, Arithmetic auto D>
		inline constexpr double ratio_leaked() noexcept
		{
			return static_cast<double>(N) / static_cast<double>(D);
		}

		template <intmax_t N1, intmax_t D1, intmax_t N2, intmax_t D2>
		inline constexpr double ratio_leaked(const std::ratio<N1, D1>& ratio1, const std::ratio<N2, D2>& ratio2) noexcept
		{
			return static_cast<double>(std::ratio<N1, D1>::num * std::ratio<N2, D2>::num) / static_cast<double>(std::ratio<N1, D1>::den * std::ratio<N2, D2>::den);
		}

		template <intmax_t N1, intmax_t D1, intmax_t N2, intmax_t D2>
		inline constexpr double ratio_leaked() noexcept
		{
			return static_cast<double>(N1 * N2) / static_cast<double>(D1 * D2);
		}
	}

	// 서버의 1초 당 프레임 수
	constexpr auto SERVER_FRAMERATE = detail::Framerate{};
	// 서버의 프레임 당 시간
	constexpr auto SERVER_TICK = detail::Tick{};

	// 최대 방의 수
	constexpr unsigned int MAX_ROOMS = 1000;
	// 방 마다의 최대 플레이어의 수 (방 인원 제한)
	constexpr unsigned int MAX_PLAYERS_PER_ROOM = 5;
	// 방 마다의 최대 NPC의 수 (NPC 수 제한)
	constexpr unsigned int MAX_NPCS_PER_ROOM = 30;

	// 최대 전체 플레이어의 수
	constexpr unsigned int MAX_USERS = MAX_ROOMS * MAX_PLAYERS_PER_ROOM;
	constexpr unsigned int USERS_ID_BEGIN = 0;

	// 최대 전체 NPC의 수
	constexpr unsigned int MAX_NPCS = MAX_ROOMS * MAX_NPCS_PER_ROOM;
	constexpr unsigned int NPC_ID_BEGIN = MAX_USERS;

	// 최대 전체 엔티티의 수
	constexpr unsigned int MAX_ENTITIES = MAX_USERS + MAX_NPCS;

	// 최대 매치의 수
	constexpr unsigned int MAX_SEARCHS = 500;

	enum class Operations : unsigned char
	{
		NONE = 0,

		ACCEPT,
		RECV,
		SEND,
		DISPOSE,

		DB_OVERLAPPED,
		DB_FIND_USER,
		DB_ADD_USER,


		ENTITY_MOVE,
	};

	enum class SessionStates : unsigned char
	{
		NONE = 0,

		ACCEPTED,
		CONNECTED,

		ROOMS = 100,
		ROOM_MAIN,
		ROOM_LOBBY,
		ROOM_INGAME,
		ROOM_COMPLETE,

		NPCS = 200,
		NPC_INGAME = ROOM_INGAME,
		NPC_DEAD = 201,
	};

	enum class RoomStates : unsigned char
	{
		IDLE = 0,
		IN_LOBBY,
		IN_READY,
		IN_GAME,
		IN_COMPLETE,
	};

	enum class SIGNIN_CAUSE : unsigned char
	{
		FAILURE_UNKNOWN_ERROR = 0, //알 수 없는 오류
		FAILURE_USER_EXCEED, // 접속한 유저 수 초과
		FAILURE_WRONG_SINGIN_INFOS, // 잘못된 ID나 비밀번호 입력으로 인한 로그인 실패
		SUCCEED = std::numeric_limits<unsigned char>::max(),
	};

	enum class SIGNUP_CAUSE : unsigned char
	{
		FAILURE_UNKNOWN_ERROR = 0, // 알 수 없는 오류
		FAILURE_DB_ERROR, // DB 서버 오류
		FAILURE_USER_DUPLICATED, // ID 중복
		SUCCEED = std::numeric_limits<unsigned char>::max(),
	};

	enum class GameInputs : unsigned char
	{
		MOVE_LT, MOVE_RT, MOVE_UP, MOVE_DW, // WASD 키
		ATTACK, // 마우스 왼쪽 클릭
		ACTION, // 마우스 오른쪽 클릭
		SUBACTION, // E 또는 F 키
		JUMP, // 스페이스 키
		SIGHT_TILT, // 마우스로 시야 전환 (roll-yaw-pitch)
	};

	enum class ObjectTags : unsigned char // 충돌 식별 태그 (유니티 태그 아님)
	{
		NONE = 0,


	};

	enum class CollisionLayers : unsigned char // 유니티 레이어
	{
		NONE = 0,


	};

	enum class DatabaseTasks : unsigned char
	{
		NONE = 0,

		FIND_USER,
		SIGNIN,
		SIGNUP,
		SIGNOUT,
	};

	class SCPacketServerInfo;
	class SCPacketSignInSucceed;
	class SCPacketSignInFailed;
	class SCPacketSignUpSucceed;
	class SCPacketSignUpFailed;

	class CSPacketSignIn;
	class CSPacketSignUp;
}
#endif // ! __PCH__
