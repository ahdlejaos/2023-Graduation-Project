#include "pch.hpp"
#include "Protocol.hpp"

namespace srv
{
#pragma pack(push, 1)
	struct BasicPacket
	{
	public:
		constexpr BasicPacket()
			: myProtocol(), mySize(sizeof(BasicPacket))
		{}

		constexpr BasicPacket(const Protocol type, const short size)
			: myProtocol(type), mySize(size)
		{}

		constexpr virtual ~BasicPacket()
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
		const short mySize;
	};
#pragma pack(pop)
}
