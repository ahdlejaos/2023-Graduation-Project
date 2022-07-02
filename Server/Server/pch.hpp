#pragma once

#ifndef  __PCH__
#define __PCH__
#include "stdafx.hpp"

class Framework;
class Session;
class PlayingSession;

class Asynchron;
class Packet;

class GameObject;
class GameEntity;
class GameTerrain;

class Player;

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
constexpr unsigned int ENTITIES_MAX = MAX_USERS + MAX_NPCS;

// �ִ� ��ġ�� ��
constexpr unsigned int MAX_SEARCHS = 500;

extern unsigned int numberRooms = 0;

#endif // ! __PCH__
