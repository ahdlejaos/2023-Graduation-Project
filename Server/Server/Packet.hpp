#pragma once

namespace srv
{
	class BasisPacket
	{
	public:
		constexpr BasisPacket(const Protocol type, const std::uint32_t size)
			: myProtocol(type), mySize(size)
		{}

		constexpr virtual ~BasisPacket()
		{}

		inline constexpr const auto& GetProtocol() const noexcept
		{
			return myProtocol;
		}

		inline constexpr const auto& GetSize() const noexcept
		{
			return mySize;
		}

	protected:
		const Protocol myProtocol;
		const std::uint32_t mySize;
	};

	template <class Derived>
		requires std::is_class_v<Derived>&& std::same_as<Derived, std::remove_cv_t<Derived>>
	class Packet : public BasisPacket
	{
	protected:
		[[nodiscard]] constexpr Derived& Cast() noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<Derived&>(*this);
		}

		[[nodiscard]] constexpr const Derived& Cast() const noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<const Derived&>(*this);
		}

	public:
		constexpr Packet(const Protocol type, const std::uint32_t size)
			: BasisPacket(type, size)
		{}

		constexpr Packet(Protocol type)
			: Packet(type, sizeof(Derived))
		{}

		constexpr virtual ~Packet()
		{}

		friend inline constexpr Derived* CreatePacket(const Derived&);
		friend inline constexpr Derived* CreatePacket(Derived&&);

		template<typename... Args>
		friend inline constexpr Derived* CreatePacket(Args&&... _Args)
			requires std::constructible_from<Derived, Args...>;

		template<typename... Args>
		friend Derived CreateLocalPacket(Args&&... _Args);
	};

	template <class Pk>
	concept packets = std::is_class_v<Pk> && std::derived_from<Pk, Packet<Pk>>;

	class SCPacketServerInfo : public Packet<SCPacketServerInfo>
	{
	public:
		constexpr SCPacketServerInfo(unsigned users, unsigned max_users, const std::span<const wchar_t> version)
			: Packet(Protocol::SC_SERVER_INFO)
			, usersCount(users), usersMax(max_users)
			, gameVersion()
		{
			std::copy(version.begin(), version.end(), std::begin(gameVersion));
		}

		unsigned usersCount, usersMax;
		wchar_t gameVersion[16];
	};

	class SCPacketSignInSucceed : public Packet<SCPacketSignInSucceed>
	{
	public:
		constexpr SCPacketSignInSucceed(unsigned long long id)
			: Packet(Protocol::SC_SIGNIN_SUCCESS)
			, userID(id)
		{}

		const PID userID;
	};

	class SCPacketSignInFailed : public Packet<SCPacketSignInFailed>
	{
	public:
		constexpr SCPacketSignInFailed(SIGNIN_CAUSE cause)
			: Packet(Protocol::SC_SIGNIN_FAILURE)
			, myCause(cause)
		{}

		const SIGNIN_CAUSE myCause;
	};

	class SCPacketSignUpSucceed : public Packet<SCPacketSignUpSucceed>
	{
	public:
		constexpr SCPacketSignUpSucceed(SIGNUP_CAUSE cause)
			: Packet(Protocol::SC_SIGNUP_SUCCESS)
			, myCause(cause)
		{}

		const SIGNUP_CAUSE myCause;
	};

	class SCPacketSignUpFailed : public Packet<SCPacketSignUpFailed>
	{
	public:
		constexpr SCPacketSignUpFailed(SIGNUP_CAUSE cause)
			: Packet(Protocol::SC_SIGNUP_FAILURE)
			, myCause(cause)
		{}

		const SIGNUP_CAUSE myCause;
	};

	class SCPacketRoomCreated : public Packet<SCPacketRoomCreated>
	{
	public:
		constexpr SCPacketRoomCreated(
			const std::size_t room_place,
			const std::wstring_view room_title
		)
			: Packet(Protocol::SC_ROOM_CREATED)
			, roomPlace(room_place), roomTitle()
		{
			std::copy(room_title.begin(), room_title.end(), roomTitle);
		}

		const std::size_t roomPlace;
		wchar_t roomTitle[30];
	};

	class SCPacketRoomCreated : public Packet<SCPacketRoomCreated>
	{
	public:
		constexpr SCPacketRoomCreated(
			const std::size_t room_place,
			const std::wstring_view room_title
		)
			: Packet(Protocol::SC_ROOM_CREATED)
			, roomPlace(room_place), roomTitle()
		{
			std::copy(room_title.begin(), room_title.end(), roomTitle);
		}

		const std::size_t roomPlace;
		wchar_t roomTitle[30];
	};

	class SCPacketRoomEntered : public Packet<SCPacketRoomEntered>
	{
	public:
		constexpr SCPacketRoomEntered(
			const std::size_t room_place
		)
			: Packet(Protocol::SC_ROOM_ENTERED)
			, roomPlace(room_place)
		{}

		const std::size_t roomPlace;
	};
	/// <summary>
	/// �� �ı� ��Ŷ (�׳� ���� ���������� �˸��� �뵵)
	/// </summary>
	class SCPacketDestroyRoom : public Packet<SCPacketDestroyRoom>
	{
	public:
		constexpr SCPacketDestroyRoom(
			const std::size_t room_place
		)
			: Packet(Protocol::SC_ROOM_DESTROYED)
			, room_id(room_place)
		{}

		std::size_t room_id;
	};

	/// <summary>
	/// �α��� ��Ŷ
	/// </summary>
	class CSPacketSignIn : public Packet<CSPacketSignIn>
	{
	public:
		/// <param name="user_id">������� ���̵� �Ǵ� ���ڸ��� �ּ� (����)</param>
		/// <param name="user_pw">������� ��й�ȣ, ��ȣȭ��</param>
		constexpr CSPacketSignIn(
			const std::wstring_view user_account,
			const std::wstring_view user_pw
		)
			: Packet(Protocol::CS_SIGNIN)
			, userAccount(), userPN()
		{
			std::copy(user_account.begin(), user_account.end(), userAccount);
			std::copy(user_pw.begin(), user_pw.end(), userPN);
		}

		wchar_t userAccount[30];
		wchar_t userPN[30];
	};

	/// <summary>
	/// ���� ��Ŷ
	/// </summary>
	class CSPacketSignUp : public Packet<CSPacketSignUp>
	{
	public:
		/// <param name="user_email">������� ���ڸ��� �ּ� (����)</param>
		/// <param name="user_pw">������� ��й�ȣ, ��ȣȭ��</param>
		/// <param name="user_nick">������� ����</param>
		constexpr CSPacketSignUp(
			const std::wstring_view user_email,
			const std::wstring_view user_pw,
			const std::wstring_view user_nick
		)
			: Packet(Protocol::CS_SIGNUP)
			, userMail(), userPN(), userName()
		{
			std::copy(user_email.begin(), user_email.end(), userMail);
			std::copy(user_pw.begin(), user_pw.end(), userPN);
			std::copy(user_nick.begin(), user_nick.end(), userName);
		}

		wchar_t userMail[30];
		wchar_t userPN[30];
		wchar_t userName[10];
	};

	/// <summary>
	/// �� ���� ��Ŷ
	/// </summary>
	class CSPacketCreateRoom : public Packet<CSPacketCreateRoom>
	{
	public:
		constexpr CSPacketCreateRoom(
			const std::size_t user_id,
			const std::wstring_view room_title
		)
			: Packet(Protocol::CS_CREATE_A_ROOM)
			, userID(user_id), roomTitle()
		{
			std::copy(room_title.begin(), room_title.end(), roomTitle);
		}

		std::size_t userID;
		wchar_t roomTitle[30];
	};

	/// <summary>
	/// �� ���� ��Ŷ
	/// </summary>
	class CSPacketJoinRoom : public Packet<CSPacketJoinRoom>
	{
	public:
		/// <param name="user_email">������� ���ڸ��� �ּ� (����)</param>
		/// <param name="user_pw">������� ��й�ȣ, ��ȣȭ��</param>
		/// <param name="user_nick">������� ����</param>
		constexpr CSPacketJoinRoom(
			const std::size_t room_place
		)
			: Packet(Protocol::CS_JOIN_A_ROOM)
			, roomID(room_place)
		{}

		std::size_t roomID;
	};

	/// <summary>
	/// �� ���� ��Ŷ
	/// </summary>
	class CSPacketMasterRoom : public Packet<CSPacketMasterRoom>
	{
	public:
		/// <param name="user_email">������� ���ڸ��� �ּ� (����)</param>
		/// <param name="user_pw">������� ��й�ȣ, ��ȣȭ��</param>
		/// <param name="user_nick">������� ����</param>
		constexpr CSPacketMasterRoom(
			const std::size_t room_place
		)
			: Packet(Protocol::CS_MASTER_A_ROOM)
			, room_id(room_place)
		{}

		std::size_t room_id;
	};

	template<packets Pk>
	[[nodiscard]] inline constexpr Pk* CreatePacket(const Pk& datagram)
	{
		return new Pk(datagram);
	}

	template<packets Pk>
	[[nodiscard]] inline constexpr Pk* CreatePacket(Pk&& datagram)
	{
		return new Pk(std::forward<Pk>(datagram));
	}

	template<packets Pk, typename... Args>
	[[nodiscard]] inline constexpr Pk* CreatePacket(Args&& ...args)
		requires std::constructible_from<Pk, Args...>
	{
		return new Pk(std::forward<Args>(args)...);
	}

	template<packets Pk, typename... Args>
	[[nodiscard]] inline constexpr Pk CreateLocalPacket(Args&& ...args)
	{
		return Pk(std::forward<Args>(args)...);
	}
}
