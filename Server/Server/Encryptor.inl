#pragma once
#include "pch.hpp"

class BaseEncryptor
{
public:
	constexpr BaseEncryptor() = default;
};

template<typename Derived>
	requires std::is_class_v<Derived>
		&& std::same_as<Derived, std::remove_cv_t<Derived>>
		&& std::derived_from<Derived, Encryptor<Derived>>
class Encryptor : public BaseEncryptor
{
protected:
	[[nodiscard]] let Derived& Cast() noexcept
	{
		return static_cast<Derived&>(*this);
	}

	[[nodiscard]] let const Derived& Cast() const noexcept
	{
		return static_cast<const Derived&>(*this);
	}

public:
	constexpr Encryptor() = default;

	let auto operator()(const std::string_view buffer) const noexcept -> std::string
	{
		return (this->Cast()).Encrypt(buffer);
	}

	let auto Encrypt(const std::string_view buffer) const noexcept -> std::string
	{
		return "CODE";
	}

	let auto Decrypt(const std::string_view code) const noexcept -> std::string
	{
		return "Default";
	}
};
