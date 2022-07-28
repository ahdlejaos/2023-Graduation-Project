#pragma once

namespace srv
{
	class Packet
	{
	public:
		constexpr Packet(Protocol type, std::uint32_t size)
			: myProtocol(type), mySize(size)
		{}

		constexpr Packet(Protocol type)
			: Packet(type, sizeof(Packet))
		{}

		constexpr virtual ~Packet() {}

		const Protocol myProtocol;
		const std::uint32_t mySize;
	};

	template<class Pk>
	concept packets = std::derived_from<Pk, Packet>;

	class SCPacketServerInfo : public Packet
	{
	public:
		constexpr SCPacketServerInfo(unsigned users, unsigned max_users, std::span<wchar_t, 8> version)
			: Packet(Protocol::SC_SERVER_INFO)
			, usersCount(users), usersMax(max_users)
			, gameVersion()
		{
			std::copy(version.begin(), version.end(), std::begin(gameVersion));
		}

		unsigned usersCount, usersMax;
		wchar_t gameVersion[8];
	};

	class SCPacketSignInSucceed : public Packet
	{
	public:
		constexpr SCPacketSignInSucceed(SIGNIN_CAUSE cause)
			: Packet(Protocol::SC_SIGNIN_SUCCESS)
			, myCause(cause)
		{}

		const SIGNIN_CAUSE myCause;
	};

	class SCPacketSignInFailed : public Packet
	{
	public:
		constexpr SCPacketSignInFailed(SIGNIN_CAUSE cause)
			: Packet(Protocol::SC_SIGNIN_FAILURE)
			, myCause(cause)
		{}

		const SIGNIN_CAUSE myCause;
	};

	class SCPacketSignUpSucceed : public Packet
	{
	public:
		constexpr SCPacketSignUpSucceed(SIGNUP_CAUSE cause)
			: Packet(Protocol::SC_SIGNUP_SUCCESS)
			, myCause(cause)
		{}

		const SIGNUP_CAUSE myCause;
	};

	class SCPacketSignUpFailed : public Packet
	{
	public:
		constexpr SCPacketSignUpFailed(SIGNUP_CAUSE cause)
			: Packet(Protocol::SC_SIGNUP_FAILURE)
			, myCause(cause)
		{}

		const SIGNUP_CAUSE myCause;
	};

	template<packets Pk, typename... Ty>
	inline constexpr Pk* CreatePacket(std::decay_t<Ty>&& ...args)
	{
		return new Pk(std::forward<decltype(args)>(args)...);
	}

	template<packets Pk>
	inline constexpr Pk* CreatePacket()
	{
		return new Pk();
	}
}
