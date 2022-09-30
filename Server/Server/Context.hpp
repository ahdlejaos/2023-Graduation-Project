#pragma once
#include "BasicContext.hpp"

namespace srv
{
	template<crtp Derived>
	class Context : public BasicContext
	{
	protected:
		[[nodiscard]]
		inline constexpr Derived& Cast() noexcept
		{
			static_assert(std::derived_from<Derived, Context>);
			return static_cast<Derived&>(*this);
		}

		[[nodiscard]]
		inline constexpr const Derived& Cast() const noexcept
		{
			static_assert(std::derived_from<Derived, Context>);
			return static_cast<const Derived&>(*this);
		}

	public:
		constexpr Context()
			: Context(Operations::NONE)
		{}

		constexpr Context(const Operations& op)
			: BasicContext(op)
		{}
	};
}
