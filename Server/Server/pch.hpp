#pragma once

#ifndef  __PCH__
#define __PCH__
#include "stdafx.hpp"

class Framework;
class Session;
class PlayingSession;
class Room;

class Asynchron;
class Packet;

class GameObject;
class GameEntity;
class GameTerrain;

class Player;

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

extern unsigned int numberRooms = 0;

enum class Protocol : unsigned int
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

	SC_SIGNIN_SUCCESS = 3000, // 로그인 성공 알림
	SC_SIGNIN_FAILURE, // 로그인 실패 알림

	SC_RESPOND_ROOMS, // 방 목록 응답
	SC_RESPOND_USERS, // 대기실에서 모든 유저의 목록 응답
	SC_RESPOND_VERSION, // 게임 서버의 버전 응답

	SC_ROOM_CREATED, // 새로운 방을 만들고, 입장한 것을 응답
	SC_ROOM_DESTROYED, // 자기가 속한 방이 사라졌음을 응답
	SC_ROOM_LEAVE, // 자기가 속한 방에서 나갔음을 응답

	SC_ROOM_ENTERED, // 방에 입장했음을 응답

	SC_CHAT, // 메시지 (시스템 알림, 방 대화, 대기실 대화, 1:1대화 등)
};

#endif // ! __PCH__
