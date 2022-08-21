#pragma once
#include "Encryptor.inl"

class MD5 : public Encryptor<MD5>
{
public:
	constexpr MD5() = default;

	constexpr auto Encrypt(const std::string_view buffer) const noexcept -> std::string
	{
		return "MD5";
	}
};
