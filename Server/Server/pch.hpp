#pragma once

#ifndef  __PCH__
#define __PCH__
#include "stdafx.hpp"

class Framework;
class ConnectService;
class AsyncPoolService;

class Session;
class PlayingSession;
class Room;

class Asynchron;
class Packet;

class GameObject;
class GameEntity;
class GameTerrain;

class Player;

namespace srv
{
	constexpr wchar_t GAME_VERSION[] = L"0.0.1";

	// 서버 스레드 수
	constexpr unsigned int THREADS_COUNT = 6;
	// 서버 TCP 포트
	constexpr unsigned short SERVER_PORT_TCP = 9000;
	// 서버 UDP 포트
	constexpr unsigned short SERVER_PORT_UDP = 9001;
	// 서버 식별자
	constexpr ULONG_PTR SERVER_ID = ULONG_PTR(-1);

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

	enum class Protocol : std::uint32_t
	{
		NONE = 0,

		CS_SIGNIN, // 로그인
		CS_SIGNUP, // 계정 가입
		CS_SIGNOUT, // 로그아웃
		CS_DISPOSE, // 로그아웃, 방, 게임, 대기실 나가기

		CS_REQUEST_ROOMS, // 방 목록 요청
		CS_REQUEST_USERS, // 대기실에서 모든 유저의 목록 요청
		CS_REQUEST_VERSION, // 게임 서버의 버전 요청

		CS_CREATE_A_ROOM, // 새로운 방을 만드는 요청
		CS_DESTROY_A_ROOM, // 방장이 방을 없애는 요청
		CS_LEAVE_A_ROOM, // 현재 방을 나가는 요청

		CS_PICK_A_ROOM, // 유저가 방을 선택해서 입장 요청
		CS_MATCH_A_ROOM, // 유저가 무작위 방에 입장 요청

		CS_CHAT, // 채팅 메시지

		SC_SERVER_INFO, // 서버 상태 알림
		SC_SIGNIN_SUCCESS, // 로그인 성공 알림
		SC_SIGNIN_FAILURE, // 로그인 실패 알림

		SC_RESPOND_ROOMS, // 방 목록 응답
		SC_RESPOND_USERS, // 대기실에서 모든 유저의 목록 응답
		SC_RESPOND_VERSION, // 게임 서버의 버전 응답

		SC_ROOM_CREATED, // 새로운 방을 만들고, 입장한 것을 응답
		SC_ROOM_DESTROYED, // 자기가 속한 방이 사라졌음을 응답
		SC_ROOM_LEAVE, // 자기가 속한 방에서 나갔음을 응답

		SC_ROOM_ENTERED, // 방에 입장했음을 응답

		SC_GAME_START, // 게임 시작
		SC_CREATE_VFX, // 시작 효과 생성
		SC_PLAY_SFX, // 효과음 재생
		SC_PLAY_MUSIC, // 음악 재생
		SC_ANIMATION_START, // 애니메이션 시작
		SC_CREATE_PLAYER, // 캐릭터 생성
		SC_CREATE_ENTITY, // 플레이어 캐릭터 이외의 캐릭터 생성 (적, 함정 등)
		SC_CREATE_OBJET, // 게임에 영향을 끼치지 않는 개체 생성
		SC_MOVE_CHARACTER, // 플레이어를 비롯한 캐릭터를 이동
		SC_MOVE_OBJET, // 게임에 영향을 끼치지 않는 개체를 이동
		SC_UPDATE_CHARACTER, // 플레이어를 비롯한 캐릭터의 상태 변경 (사망, 상태이상 등)
		SC_UPDATE_OBJET, // 게임에 영향을 끼치지 않는 개체의 상태 변경
		SC_REMOVE_CHARACTER, // 플레이어를 비롯한 캐릭터를 삭제
		SC_REMOVE_OBJET, // 게임에 영향을 끼치지 않는 개체를 삭제

		SC_CHAT, // 메시지 (시스템 알림, 방 대화, 대기실 대화, 1:1대화 등)
	};

	enum class Operations : unsigned char
	{
		NONE = 0,

		ACCEPT,
		RECV,
		SEND,
		DISPOSE,

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
		SUCCEED = 0,
		FAILURE_UNKNOWN_ERROR,
		FAILURE_USER_EXCEED,
		FAILURE_WRONG_SINGIN_INFOS,
		FAILURE_WRONG = std::numeric_limits<unsigned char>::max(),
	};

	enum class ObjectTags : unsigned char // 충돌 식별 태그 (유니티 태그 아님)
	{
		NONE = 0,


	};

	enum class CollisionLayers : unsigned char // 유니티 레이어
	{
		NONE = 0,


	};
}
#endif // ! __PCH__
