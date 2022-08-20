#pragma once
#include "pch.hpp"

class BaseEncryptor
{
public:
	constexpr BaseEncryptor() = default;
};

template<typename Derived>
	requires std::is_class_v<Derived>&& std::same_as<Derived, std::remove_cv_t<Derived>>
class Encryptor : public BaseEncryptor
{
protected:
	[[nodiscard]] let Derived& Cast() noexcept
	{
		static_assert(std::derived_from<Derived, Encryptor>);
		return static_cast<Derived&>(*this);
	}

	[[nodiscard]] let const Derived& Cast() const noexcept
	{
		static_assert(std::derived_from<Derived, Encryptor>);
		return static_cast<const Derived&>(*this);
	}

public:
	constexpr Encryptor() = default;

	let auto operator()() const noexcept
	{
		return (this->Cast())();
	}
};
