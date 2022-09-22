#pragma once

class DatabaseQuery
{
public:
	constexpr DatabaseQuery()
		: myStatement()
		, myQuery(NULL)
	{}

	constexpr DatabaseQuery(const SQLHSTMT query)
		: myStatement()
		, myQuery(query)
	{}

	constexpr DatabaseQuery(const std::wstring_view& statement, const SQLHSTMT query)
		: myStatement(statement)
		, myQuery(query)
	{}

	~DatabaseQuery()
	{
		if (NULL != myQuery)
		{
			Destroy();
			myQuery = NULL;
		}
	}

	bool Execute()
	{
		if (NULL != myQuery)
		{
			auto sqlcode = SQLExecute(myQuery);

			if (SQLSucceed(sqlcode))
			{
				return true;
			}
		}

		return false;
	}

	template<typename Ty>
	SQLRETURN Bind(int column, SQLSMALLINT sql_type, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql_type, place, length, result_length);
	}

	template<typename Ty>
	SQLRETURN Bind(int column, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, DeductSQLType<Ty>(), place, length, result_length);
	}

	SQLRETURN FetchOnce()
	{
		return SQLFetch(myQuery);
	}

	bool Fetch()
	{
		SQLRETURN sqlcode{};

		do
		{
			sqlcode = FetchOnce();

			if (SQLFailed(sqlcode))
			{
				break;
			}
			else if (SQLSucceedWithInfo(sqlcode))
			{

				//return true;
			}
			else if (SQLSucceed(sqlcode))
			{

				//return true;
			}
			else if (SQLFetchEnded(sqlcode))
			{
				//result = SQL_NO_DATA_FOUND;
				break;
			}
			else
			{
				break;
			}
		}
		while (SQLFetchEnded(sqlcode));
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
