#include "pch.hpp"
#include "DatabaseQuery.hpp"

bool DatabaseQuery::Execute()
{
	if (NULL != myQuery)
	{
		auto sqlcode = SQLExecute(myQuery);

		if (SQLSucceed(sqlcode))
		{
			return true;
		}
		else
		{
			SQLDiagnostics(SQL_HANDLE_STMT, myQuery);
		}
	}

	return false;
}

SQLRETURN DatabaseQuery::Fetch()
{
	SQLRETURN sqlcode{};

	do
	{
		sqlcode = FetchOnce();

		if (SQLStatementHasDiagnotics(sqlcode))
		{
			SQLDiagnostics(SQL_HANDLE_STMT, myQuery);
			break;
		}
		else if (SQL_INVALID_HANDLE == sqlcode)
		{
			SQLDiagnostics(SQL_HANDLE_STMT, myQuery);
			break;
		}
	}
	while (!SQLFetchEnded(sqlcode));

	return sqlcode;
}