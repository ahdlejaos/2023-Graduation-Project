#pragma once

namespace srv
{
	template<typename Derived> requires crtp<Derived>
	class BasicContext : public WSAOVERLAPPED
	{
	protected:
		[[nodiscard]]
		inline constexpr Derived& Cast() noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<Derived&>(*this);
		}

		[[nodiscard]]
		inline constexpr const Derived& Cast() const noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<const Derived&>(*this);
		}

	public:
		constexpr BasicContext()
			: BasicContext(Operations::NONE)
		{}

		constexpr BasicContext(const Operations& op)
			: WSAOVERLAPPED()
			, myOperation(op)
		{}

		Operations myOperation;
	};
}
