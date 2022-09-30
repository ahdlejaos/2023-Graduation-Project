#pragma once

namespace srv
{
	template<typename Derived>
		requires std::is_class_v<Derived>&& std::same_as<Derived, std::remove_cv_t<Derived>>
	class BasicContext : public WSAOVERLAPPED
	{
	protected:
		[[nodiscard]] constexpr Derived& Cast() noexcept
		{
			static_assert(std::derived_from<Derived, Packet>);
			return static_cast<Derived&>(*this);
		}

		[[nodiscard]] constexpr const Derived& Cast() const noexcept
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
