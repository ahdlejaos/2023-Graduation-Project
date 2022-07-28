#pragma once

namespace srv
{
	template <class Derived>
		requires std::is_class_v<Derived> &&std::same_as<Derived, std::remove_cv_t<Derived>>
	class Packet
	{
	public:
		constexpr Packet(Protocol type, std::uint32_t size)
			: myProtocol(type), mySize(size)
		{}

		constexpr Packet(Protocol type)
			: Packet(type, sizeof(Derived))
		{}

		constexpr virtual ~Packet() {}

		const Protocol myProtocol;
		const std::uint32_t mySize;

	protected:
		[[nodiscard]] constexpr Derived &Cast() noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<Derived &>(*this);
		}

		[[nodiscard]] constexpr const Derived &Cast() const noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<const Derived &>(*this);
		}
	};

	template <class Pk>
	concept packets = std::derived_from<Pk, Packet<Pk>> && std::is_class_v<Pk>;

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

	template<packets Pk>
	inline constexpr Pk *CreatePacket(Pk &&datagram)
	{
		return new Pk(std::forward<Pk>(datagram));
	}

	template<packets Pk>
	inline constexpr Pk *CreatePacket()
	{
		return new Pk();
	}

	template<packets Pk, typename... Ty>
	inline constexpr Pk *CreatePacket(Ty&& ...args)
	{
		return new Pk(std::forward<decltype(args)>(args)...);
	}
}
