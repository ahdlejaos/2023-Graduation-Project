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
	// ���� ������ ��
	constexpr unsigned int THREADS_COUNT = 6;
	// ���� TCP ��Ʈ
	constexpr unsigned int SERVER_PORT_TCP = 9000;
	// ���� UDP ��Ʈ
	constexpr unsigned int SERVER_PORT_UDP = 9001;
	// ���� �ĺ���
	constexpr ULONG_PTR SERVER_ID = ULONG_PTR(-1);

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

		SC_GAME_START = 5000, // ���� ����
		SC_CREATE_VFX, // ���� ȿ�� ����
		SC_PLAY_SFX, // ȿ���� ���
		SC_PLAY_MUSIC, // ���� ���
		SC_ANIMATION_START, // �ִϸ��̼� ����
		SC_CREATE_PLAYER, // ĳ���� ����
		SC_CREATE_ENTITY, // �÷��̾� ĳ���� �̿��� ĳ���� ���� (��, ���� ��)
		SC_CREATE_OBJET, // ���ӿ� ������ ��ġ�� �ʴ� ��ü ����
		SC_MOVE_CHARACTER, // �÷��̾ ����� ĳ���͸� �̵�
		SC_MOVE_OBJET, // ���ӿ� ������ ��ġ�� �ʴ� ��ü�� �̵�
		SC_UPDATE_CHARACTER, // �÷��̾ ����� ĳ������ ���� ���� (���, �����̻� ��)
		SC_UPDATE_OBJET, // ���ӿ� ������ ��ġ�� �ʴ� ��ü�� ���� ����
		SC_REMOVE_CHARACTER, // �÷��̾ ����� ĳ���͸� ����
		SC_REMOVE_OBJET, // ���ӿ� ������ ��ġ�� �ʴ� ��ü�� ����

		SC_CHAT, // �޽��� (�ý��� �˸�, �� ��ȭ, ���� ��ȭ, 1:1��ȭ ��)
	};

	enum class Operations : unsigned char
	{
		NONE = 0,

		ACCEPT,
		RECV,
		SEND,

		ENTITY_MOVE,
	};

	enum class SessionStates : unsigned char
	{
		NONE = 0,

		CONNECTED,
		ACCEPTED,

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

	enum class ObjectTags : unsigned char // �浹 �ĺ� �±� (����Ƽ �±� �ƴ�)
	{
		NONE = 0,


	};

	enum class CollisionLayers : unsigned char // ����Ƽ ���̾�
	{
		NONE = 0,


	};
}
#endif // ! __PCH__
