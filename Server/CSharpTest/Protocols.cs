using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpTest
{
	static class Server
	{
		public const short SERVER_PORT = 12000;
	}

	internal class Protocols
	{

	}

	public enum Protocol : Int32
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
		CS_INPUT, // 게임 조작 입력

		SC_SERVER_INFO, // 서버 상태 알림
		SC_SIGNIN_SUCCESS, // 로그인 성공 알림
		SC_SIGNIN_FAILURE, // 로그인 실패 알림
		SC_SIGNUP_SUCCESS, // 가입 성공 알림
		SC_SIGNUP_FAILURE, // 가입 실패 알림

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
	}
}
