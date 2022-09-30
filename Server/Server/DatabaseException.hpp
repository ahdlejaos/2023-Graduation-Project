#pragma once

class DatabaseException : public std::exception
{
public:
	DatabaseException(std::string_view what)
		: exception(what.data())
	{}
};
