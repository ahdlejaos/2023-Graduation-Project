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
		constexpr SCPacketSignInSucceed(SIGNIN_CAUSE cause)
			: Packet(Protocol::SC_SIGNIN_SUCCESS)
			, myCause(cause)
		{}

		const SIGNIN_CAUSE myCause;
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

	/// <summary>
	/// 로그인 패킷
	/// </summary>
	class CSPacketSignIn : public Packet<CSPacketSignIn>
	{
	public:
		/// <param name="user_id">사용자의 아이디 또는 전자메일 주소 (유일)</param>
		/// <param name="user_pw">사용자의 비밀번호, 부호화됨</param>
		constexpr CSPacketSignIn(
			const std::span<wchar_t, 30> user_id,
			const std::span<wchar_t, 30> user_pw
		)
			: Packet(Protocol::CS_SIGNIN)
			, userID(), userPN()
		{
			std::copy(user_id.begin(), user_id.end(), userID);
			std::copy(user_pw.begin(), user_pw.end(), userPN);
		}

		wchar_t userID[30];
		wchar_t userPN[30];
	};

	/// <summary>
	/// 가입 패킷
	/// </summary>
	class CSPacketSignUp : public Packet<CSPacketSignUp>
	{
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="user_email">사용자의 전자메일 주소 (유일)</param>
		/// <param name="user_pw">사용자의 비밀번호, 부호화됨</param>
		/// <param name="user_nick">사용자의 별명</param>
		constexpr CSPacketSignUp(
			const std::span<wchar_t, 30> user_email,
			const std::span<wchar_t, 30> user_pw,
			const std::span<wchar_t, 10> user_nick
		)
			: Packet(Protocol::CS_SIGNUP)
			, userID(), userPN()
		{
			std::copy(user_email.begin(), user_email.end(), userID);
			std::copy(user_pw.begin(), user_pw.end(), userPN);
			std::copy(user_nick.begin(), user_nick.end(), userName);
		}

		wchar_t userID[30];
		wchar_t userPN[30];
		wchar_t userName[10];
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
