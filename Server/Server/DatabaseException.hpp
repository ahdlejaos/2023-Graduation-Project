#pragma once
#include "MySQL.hpp"

namespace db
{
	class Exception : public std::exception
	{
	public:
		Exception(const char* const what, std::wstring_view id, std::wstring_view qmsg, SQLINTEGER code)
			: exception(what)
			, msg{ qmsg.cbegin(), qmsg.cend() }
			, state{ id.cbegin(), id.cend() }
			, native(code)
		{}

		const SQLINTEGER native;
		const std::wstring msg;
		const std::wstring state;
	};
}

inline void RaiseDatabaseError(const char* const what) noexcept(false)
{
	throw db::Exception{ what, sql::state, sql::msg, sql::native };
}
