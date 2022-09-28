#pragma once

template<std::integral Elem, size_t OutputLength>
	requires requires(std::basic_string<Elem> value)
{
	std::char_traits<Elem>{};
	value = "";
}


class StringHasher
{
public:
	std::array<Elem, OutputLength> myOutput;
	std::array<std::array<Elem, OutputLength>, 2> myStorages;
	std::array<Elem, OutputLength * 2> myComfy;
};
