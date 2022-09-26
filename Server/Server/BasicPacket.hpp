#include "pch.hpp"

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
