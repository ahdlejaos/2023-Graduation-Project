namespace srv
{
	enum class Protocol : std::uint32_t
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
		CS_JOIN_A_ROOM, // ���� ���� ��û
		CS_LEAVE_A_ROOM, // ���� ���� ������ ��û
		CS_MASTER_A_ROOM, // ���� ���� ������ �Ǵ� ��û

		CS_PICK_A_ROOM, // ������ ���� �����ؼ� ���� ��û
		CS_MATCH_A_ROOM, // ������ ������ �濡 ���� ��û

		CS_CHAT, // ä�� �޽���
		CS_INPUT, // ���� ���� �Է�

		SC_SERVER_INFO, // ���� ���� �˸�
		SC_SIGNIN_SUCCESS, // �α��� ���� �˸�
		SC_SIGNIN_FAILURE, // �α��� ���� �˸�
		SC_SIGNUP_SUCCESS, // ���� ���� �˸�
		SC_SIGNUP_FAILURE, // ���� ���� �˸�

		SC_RESPOND_ROOMS, // �� ��� ����
		SC_RESPOND_USERS, // ���ǿ��� ��� ������ ��� ����
		SC_RESPOND_VERSION, // ���� ������ ���� ����

		SC_ROOM_CREATED, // ���ο� ���� �����, ������ ���� ����
		SC_ROOM_DESTROYED, // �ڱⰡ ���� ���� ��������� ����
		SC_ROOM_ENTERED, // �濡 ���������� ����
		SC_ROOM_LEAVE, // �ڱⰡ ���� �濡�� �������� ����

		SC_GAME_START, // ���� ����
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
}
