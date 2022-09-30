#pragma once

namespace srv
{
	class BasicContext : public WSAOVERLAPPED
	{
	public:
		constexpr BasicContext()
			: BasicContext(Operations::NONE)
		{}

		constexpr BasicContext(const Operations& op)
			: WSAOVERLAPPED()
			, myOperation(op)
		{}

		[[nodiscard]]
		inline constexpr WSAOVERLAPPED* Flip() noexcept
		{
			return static_cast<WSAOVERLAPPED*>(this);
		}

		[[nodiscard]]
		inline constexpr const WSAOVERLAPPED* Flip() const noexcept
		{
			return static_cast<const WSAOVERLAPPED*>(this);
		}

		Operations myOperation;
	};
}
