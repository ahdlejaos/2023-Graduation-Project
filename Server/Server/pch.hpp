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
	CS_SIGNIN = 0, // 로그인
	CS_SIGNUP, // 계정 가입
	CS_SIGNOUT, // 로그아웃



	SC_SIGNIN_SUCCESS = 1000, // 로그인 성공 알림
	SC_SIGNIN_FAILURE, // 로그인 실패 알림

};

#endif // ! __PCH__
