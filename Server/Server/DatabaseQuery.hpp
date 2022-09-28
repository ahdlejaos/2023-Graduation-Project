#pragma once
#include "MySQL.hpp"

class DatabaseQuery
{
public:
	DatabaseQuery()
		: myStatement(), myQuery(NULL)
		, myBackend(), isEnded(false)
	{}

	DatabaseQuery(const std::wstring_view& statement)
		: myStatement(statement), myQuery(NULL)
		, myBackend(), isEnded(false)
	{}

	~DatabaseQuery()
	{
		if (NULL != myQuery)
		{
			Destroy();
			myQuery = NULL;
		}
	}

	SQLRETURN Execute();

	SQLRETURN Fetch();

	template<typename Ty>
	inline SQLRETURN Bind(int column, SQLSMALLINT sql_type, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql_type, place, length, result_length);
	}

	template<typename Ty>
	inline SQLRETURN Bind(int column, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql::Deduct<Ty>(), place, length, result_length);
	}

	inline SQLRETURN FetchOnce()
	{
		return SQLFetch(myQuery);
	}

	inline SQLRETURN Cancel()
	{
		return SQLCancel(myQuery);
	}

	inline SQLRETURN Destroy()
	{
		return SQLFreeHandle(SQL_HANDLE_STMT, myQuery);
	}

	std::wstring myStatement;
	SQLHSTMT myQuery;

	std::promise<void> myBackend;

	bool isEnded;
};
