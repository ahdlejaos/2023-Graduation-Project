#pragma once
#include "Encryptor.inl"

class MD5 : public Encryptor<MD5>
{
public:
	constexpr MD5() = default;

	let auto operator()() const noexcept
	{

	}
};
