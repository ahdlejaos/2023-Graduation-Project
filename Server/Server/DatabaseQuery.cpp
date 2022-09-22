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
