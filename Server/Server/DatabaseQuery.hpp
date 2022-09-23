#pragma once
#include "MySQL.hpp"

class DatabaseQuery
{
public:
	constexpr DatabaseQuery()
		: myStatement()
		, myQuery(NULL)
	{}

	constexpr DatabaseQuery(const SQLHSTMT& query)
		: myStatement()
		, myQuery(query)
	{}

	constexpr DatabaseQuery(const std::wstring& statement, const SQLHSTMT& query)
		: myStatement(statement)
		, myQuery(query)
	{}

	constexpr DatabaseQuery(std::wstring&& statement)
		: myStatement(std::forward<std::wstring>(statement))
		, myQuery()
	{}

	~DatabaseQuery()
	{
		if (NULL != myQuery)
		{
			Destroy();
			myQuery = NULL;
		}
	}

	bool Execute();

	SQLRETURN Fetch();

	template<typename Ty>
	SQLRETURN Bind(int column, SQLSMALLINT sql_type, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql_type, place, length, result_length);
	}

	template<typename Ty>
	SQLRETURN Bind(int column, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql::Deduct<Ty>(), place, length, result_length);
	}

	SQLRETURN FetchOnce()
	{
		return SQLFetch(myQuery);
	}

	SQLRETURN Cancel()
	{
		return SQLCancel(myQuery);
	}

	SQLRETURN Destroy()
	{
		return SQLFreeHandle(SQL_HANDLE_STMT, myQuery);
	}

	std::wstring myStatement;
	SQLHSTMT myQuery;

	bool isEnded = false;
};
