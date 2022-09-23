#include "stdafx.hpp"
#include <sql.h>
#include <sqlext.h>

#ifndef __SQL_UTILITIES__
#define __SQL_UTILITIES__

extern "C" let bool SQLSucceed(const SQLRETURN code) noexcept
{
	return (SQL_SUCCEEDED(code));
}

extern "C" let bool SQLSucceedWithInfo(const SQLRETURN code) noexcept
{
	return (code == SQL_SUCCESS_WITH_INFO);
}

extern "C" let bool SQLStatementHasDiagnotics(const SQLRETURN code) noexcept
{
	return (code == SQL_ERROR || code == SQL_SUCCESS_WITH_INFO);
}

extern "C" let bool SQLDiagEmpty(const SQLRETURN code) noexcept
{
	return (code == SQL_NO_DATA_FOUND);
}

extern "C" let bool SQLFetchEnded(const SQLRETURN code) noexcept
{
	return (code == SQL_NO_DATA_FOUND);
}

extern "C" let bool SQLHasParameters(const SQLRETURN code) noexcept
{
	return (code == SQL_PARAM_DATA_AVAILABLE);
}

extern "C" let bool SQLFailed(const SQLRETURN code) noexcept
{
	return (code == SQL_ERROR);
}

extern "C" inline SQLRETURN SQLDiagnostics(const SQLSMALLINT & type, const SQLHANDLE & target)
{
	static SQLSMALLINT records = 0;
	static SQLINTEGER native{};
	static SQLWCHAR state[7]{};
	static SQLWCHAR msg[512]{};
	static SQLSMALLINT msg_length{};

	SQLRETURN sqlcode = SQLGetDiagRecW(type, target
		, ++records
		, state, &native
		, msg, sizeof(msg), &msg_length);

	if (!SQLDiagEmpty(sqlcode))
	{
		const std::wstring temp_state{ state, state + 7 };
		const std::wstring temp_msg{ msg, msg + 256 };

		std::wcout << "state: " << temp_state << "\nmsg(" << native << "): " << temp_msg << '\n';
	}

	return sqlcode;
}

namespace sql
{
	namespace detail
	{
		template<typename Ty>
		struct SQLTypeFilter
		{};

		template<>
		struct SQLTypeFilter<char[]>
		{
			static constexpr SQLSMALLINT value = SQL_C_CHAR;
		};

		template<>
		struct SQLTypeFilter<wchar_t[]>
		{
			static constexpr SQLSMALLINT value = SQL_C_WCHAR;
		};

		template<size_t Size>
		struct SQLTypeFilter<char[Size]>
		{
			static constexpr SQLSMALLINT value = SQL_C_CHAR;
		};

		template<size_t Size>
		struct SQLTypeFilter<wchar_t[Size]>
		{
			static constexpr SQLSMALLINT value = SQL_C_WCHAR;
		};

		template<>
		struct SQLTypeFilter<SQLWCHAR>
		{
			static constexpr SQLSMALLINT value = SQL_C_WCHAR;
		};

		template<>
		struct SQLTypeFilter<char*>
		{
			static constexpr SQLSMALLINT value = SQL_C_CHAR;
		};

		template<>
		struct SQLTypeFilter<wchar_t*>
		{
			static constexpr SQLSMALLINT value = SQL_C_WCHAR;
		};

		template<>
		struct SQLTypeFilter<void*>
		{
			static constexpr SQLSMALLINT value = SQL_BINARY;
		};

		template<>
		struct SQLTypeFilter<SQLSCHAR>
		{
			static constexpr SQLSMALLINT value = SQL_C_STINYINT;
		};

		template<>
		struct SQLTypeFilter<SQLCHAR>
		{
			static constexpr SQLSMALLINT value = SQL_C_BIT;
		};

		template<>
		struct SQLTypeFilter<int>
		{
			static constexpr SQLSMALLINT value = SQL_INTEGER;
		};

		template<>
		struct SQLTypeFilter<float>
		{
			static constexpr SQLSMALLINT value = SQL_C_FLOAT;
		};

		template<>
		struct SQLTypeFilter<SQLFLOAT> // SQLDOUBLE
		{
			static constexpr SQLSMALLINT value = SQL_C_FLOAT;
		};

		template<>
		struct SQLTypeFilter<SQLSMALLINT>
		{
			static constexpr SQLSMALLINT value = SQL_C_SSHORT;
		};

		template<>
		struct SQLTypeFilter<SQLUSMALLINT>
		{
			static constexpr SQLSMALLINT value = SQL_C_USHORT;
		};

		template<>
		struct SQLTypeFilter<SQLINTEGER>
		{
			static constexpr SQLSMALLINT value = SQL_C_SLONG;
		};

		template<>
		struct SQLTypeFilter<SQLUINTEGER>
		{
			static constexpr SQLSMALLINT value = SQL_C_ULONG;
		};
	}

	template<typename Ty>
		requires requires {
		detail::SQLTypeFilter<Ty>{};
	}
	inline constexpr auto Deduct() -> SQLSMALLINT
	{
		return detail::SQLTypeFilter<Ty>::value;
	}

	template<typename Ty>
	constexpr SQLSMALLINT DeductSQLType()
	{
		using PureTy = std::remove_cvref_t<Ty>;
		using NotArray = std::remove_all_extents_t<PureTy>;

		if constexpr (std::is_array_v<Ty>)
		{
			if constexpr (std::is_base_of_v<char, PureTy>)
			{
				return SQL_C_CHAR;
			}
			else if constexpr (std::is_base_of_v<wchar_t, PureTy>)
			{
				return SQL_C_WCHAR;
			}
		}
		else if constexpr (std::is_floating_point_v<PureTy>)
		{
			if constexpr (std::is_same_v<PureTy, double>)
			{
				return SQL_C_DOUBLE;
			}
			else
			{
				return SQL_C_FLOAT;
			}
			return SQL_C_LONG;
		}
		else
		{
			if constexpr (std::is_same_v<PureTy, short>)
			{
				return SQL_C_SHORT;
			}
			else if constexpr (std::is_same_v<PureTy, unsigned short>)
			{
				return SQL_C_USHORT;
			}
			else if constexpr (std::is_unsigned_v<PureTy>)
			{
				return SQL_C_ULONG;
			}
			else
			{
				return SQL_C_LONG;
			}
		}

		return 0;
	}
}


#endif
