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

extern unsigned int numberRooms = 0;

enum class Protocol : unsigned int
{
	NONE = 0,

	CS_SIGNIN, // �α���
	CS_SIGNUP, // ���� ����
	CS_SIGNOUT, // �α׾ƿ�
	CS_DISPOSE, // �α׾ƿ�, ��, ����, ���� ������

	CS_REQUEST_ROOMS, // �� ��� ��û
	CS_REQUEST_USERS, // ���ǿ��� ��� ������ ��� ��û
	CS_REQUEST_VERSION, // ���� ������ ���� ��û

	CS_CREATE_A_ROOM, // ���ο� ���� ����� ��û
	CS_DESTROY_A_ROOM, // ������ ���� ���ִ� ��û
	CS_LEAVE_A_ROOM, // ���� ���� ������ ��û

	CS_PICK_A_ROOM, // ������ ���� �����ؼ� ���� ��û
	CS_MATCH_A_ROOM, // ������ ������ �濡 ���� ��û

	CS_CHAT, // ä�� �޽���

	SC_SIGNIN_SUCCESS = 3000, // �α��� ���� �˸�
	SC_SIGNIN_FAILURE, // �α��� ���� �˸�

	SC_RESPOND_ROOMS, // �� ��� ����
	SC_RESPOND_USERS, // ���ǿ��� ��� ������ ��� ����
	SC_RESPOND_VERSION, // ���� ������ ���� ����

	SC_ROOM_CREATED, // ���ο� ���� �����, ������ ���� ����
	SC_ROOM_DESTROYED, // �ڱⰡ ���� ���� ��������� ����
	SC_ROOM_LEAVE, // �ڱⰡ ���� �濡�� �������� ����

	SC_ROOM_ENTERED, // �濡 ���������� ����

	SC_CHAT, // �޽��� (�ý��� �˸�, �� ��ȭ, ���� ��ȭ, 1:1��ȭ ��)
};

#endif // ! __PCH__
