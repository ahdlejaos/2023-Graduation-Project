#include "pch.hpp"
#include "DatabaseQuery.hpp"

SQLRETURN db::Query::Execute()
{
	SQLRETURN sqlcode{};

	if (NULL != myQuery)
	{
		sqlcode = SQLExecute(myQuery);

		if (sql::IsSucceed(sqlcode))
		{
			return true;
		}
		else
		{
			sql::GrabDiagnostics(SQL_HANDLE_STMT, myQuery);
		}
	}

	return sqlcode;
}

SQLRETURN db::Query::Fetch()
{
	SQLRETURN sqlcode{};

	do
	{
		sqlcode = FetchOnce();

		if (sql::IsStatementHasDiagnotics(sqlcode))
		{
			sql::GrabDiagnostics(SQL_HANDLE_STMT, myQuery);
			break;
		}
		else if (SQL_INVALID_HANDLE == sqlcode)
		{
			sql::GrabDiagnostics(SQL_HANDLE_STMT, myQuery);
			break;
		}
	}
	while (!sql::IsFetchEnded(sqlcode));

	return sqlcode;
}
