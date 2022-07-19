#pragma once

namespace srv
{
	class Packet
	{
	public:
		constexpr Packet(Protocol type, unsigned size)
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

	class SCPacketSignUp : public Packet
	{
	public:
		constexpr SCPacketSignUp()
			: Packet(srv::Protocol::SC_SIGNIN_SUCCESS)
		{}
	};

	template<packets Pk, typename... Ty>
	inline constexpr Pk* CreatePacket(std::remove_cvref_t<Ty>&& ...args)
	{
		return new Pk(std::forward<Ty>(args)...);
	}

	template<packets Pk>
	inline constexpr Pk* CreatePacket()
	{
		return new Pk();
	}
}
