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

	// ���� ������ ��
	constexpr unsigned int THREADS_COUNT = 6;
	// ���� TCP ��Ʈ
	constexpr unsigned short SERVER_PORT_TCP = 12000;
	// ���� UDP ��Ʈ
	constexpr unsigned short SERVER_PORT_UDP = 12001;
	// ���� �ĺ���
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

	// ������ 1�� �� ������ ��
	constexpr auto SERVER_FRAMERATE = detail::Framerate{};
	// ������ ������ �� �ð�
	constexpr auto SERVER_TICK = detail::Tick{};

	// �ִ� ���� ��
	constexpr unsigned int MAX_ROOMS = 1000;
	// �� ������ �ִ� �÷��̾��� �� (�� �ο� ����)
	constexpr unsigned int MAX_PLAYERS_PER_ROOM = 5;
	// �� ������ �ִ� NPC�� �� (NPC �� ����)
	constexpr unsigned int MAX_NPCS_PER_ROOM = 30;

	// �ִ� ��ü �÷��̾��� ��
	constexpr unsigned int MAX_USERS = MAX_ROOMS * MAX_PLAYERS_PER_ROOM;
	constexpr unsigned int USERS_ID_BEGIN = 0;

	// �ִ� ��ü NPC�� ��
	constexpr unsigned int MAX_NPCS = MAX_ROOMS * MAX_NPCS_PER_ROOM;
	constexpr unsigned int NPC_ID_BEGIN = MAX_USERS;

	// �ִ� ��ü ��ƼƼ�� ��
	constexpr unsigned int MAX_ENTITIES = MAX_USERS + MAX_NPCS;

	// �ִ� ��ġ�� ��
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
		FAILURE_UNKNOWN_ERROR = 0, //�� �� ���� ����
		FAILURE_USER_EXCEED, // ������ ���� �� �ʰ�
		FAILURE_WRONG_SINGIN_INFOS, // �߸��� ID�� ��й�ȣ �Է����� ���� �α��� ����
		SUCCEED = std::numeric_limits<unsigned char>::max(),
	};

	enum class SIGNUP_CAUSE : unsigned char
	{
		FAILURE_UNKNOWN_ERROR = 0, // �� �� ���� ����
		FAILURE_DB_ERROR, // DB ���� ����
		FAILURE_USER_DUPLICATED, // ID �ߺ�
		SUCCEED = std::numeric_limits<unsigned char>::max(),
	};

	enum class GameInputs : unsigned char
	{
		MOVE_LT, MOVE_RT, MOVE_UP, MOVE_DW, // WASD Ű
		ATTACK, // ���콺 ���� Ŭ��
		ACTION, // ���콺 ������ Ŭ��
		SUBACTION, // E �Ǵ� F Ű
		JUMP, // �����̽� Ű
		SIGHT_TILT, // ���콺�� �þ� ��ȯ (roll-yaw-pitch)
	};

	enum class ObjectTags : unsigned char // �浹 �ĺ� �±� (����Ƽ �±� �ƴ�)
	{
		NONE = 0,


	};

	enum class CollisionLayers : unsigned char // ����Ƽ ���̾�
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
