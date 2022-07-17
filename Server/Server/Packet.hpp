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

	template<packets Pk>
	inline constexpr Packet* CreatePacket(const Protocol& type)
	{
		return new Pk(type);
	}

	template<packets Pk, typename... _Valty>
	inline constexpr Packet* CreatePacket(const Protocol& type, _Valty ...vargs)
	{
		return new Pk(type, vargs...);
	}
}
