#include "pch.hpp"

namespace srv
{
#pragma pack(push, 1)
	struct BasicPacket
	{
	public:
		constexpr BasicPacket()
			: myProtocol(), mySize(sizeof(BasicPacket))
		{}

		constexpr BasicPacket(const Protocol type, const unsigned short size)
			: myProtocol(type), mySize(size)
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
		const unsigned short mySize;
	};
#pragma pack(pop)
}
