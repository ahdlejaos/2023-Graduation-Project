#pragma once

namespace srv
{
	template<crtp Derived>
	class BasicContext : public WSAOVERLAPPED
	{
	protected:
		[[nodiscard]]
		inline constexpr Derived& Cast() noexcept
		{
			static_assert(std::derived_from<Derived, BasicContext>);
			return static_cast<Derived&>(*this);
		}

		[[nodiscard]]
		inline constexpr const Derived& Cast() const noexcept
		{
			static_assert(std::derived_from<Derived, BasicContext>);
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
