#pragma once

class DatabaseException : public std::exception
{
public:
	DatabaseException(const char* const what, std::wstring_view id, std::wstring_view qmsg, SQLINTEGER code)
		: exception(what)
		, msg{ qmsg.cbegin(), qmsg.cend() }
		, state{ id }, native(code)
	{}

	const SQLINTEGER native;
	const std::wstring msg;
	const std::wstring state;
};

namespace sql
{
	static SQLSMALLINT records = 0;
	static SQLINTEGER native{};
	static SQLWCHAR state[7]{};
	static SQLWCHAR msg[1024]{};
	static SQLSMALLINT msg_length{};
}

inline void RaiseDatabaseError(const char* const what) noexcept(false)
{
	throw DatabaseException(what, sql::state, sql::msg, sql::native);
}
