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

	class SCPacketSignUpSucceed : public Packet
	{
	public:
		constexpr SCPacketSignUpSucceed()
			: Packet(Protocol::SC_SIGNIN_SUCCESS)
		{}
	};

	class SCPacketSignUpFailed : public Packet
	{
	public:
		constexpr SCPacketSignUpFailed()
			: Packet(Protocol::SC_SIGNIN_FAILURE)
		{}
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
