#include "pch.hpp"
#include "DatabaseService.hpp"
#include "DatabaseQuery.hpp"

DatabaseService::DatabaseService()
	: myEnvironment(NULL), myConnector(NULL)
	, isConnected(false)
{}

DatabaseService::~DatabaseService()
{
	if (isConnected)
	{
		Disconnect();
	}
}

bool DatabaseService::Awake()
{
	if (!std::filesystem::exists(mySecrets))
	{
		std::cerr << "비밀 정보 파일이 없음!\n";
		return false;
	}

	std::ifstream my_raw_secret{ mySecrets };
	if (!my_raw_secret)
	{
		std::cerr << "비밀 파일을 불러올 수 없음!\n";
		return false;
	}

	json my_secret = json::parse(my_raw_secret);
	if (!my_raw_secret)
	{
		std::cerr << "JSON에 문제가 있어서 비밀 파일을 불러올 수 없음!\n";
		return false;
	}

	const auto& db_name = (my_secret["username"]).get<std::string>();
	const auto& db_key = (my_secret["password"]).get<std::string>();

	const auto& db_wname = std::wstring{ db_name.cbegin(), db_name.cend() };
	const auto& db_wkey = std::wstring{ db_key.cbegin(), db_key.cend() };

	auto ent_wstr = myEntry.c_str();
	auto name_wstr = db_wname.c_str();
	auto pw_wstr = db_wkey.c_str();

	auto entry = const_cast<SQLWCHAR*>(ent_wstr);
	const auto entrylen = lstrlen(entry);
	auto username = const_cast<SQLWCHAR*>(name_wstr);
	const auto userlen = lstrlen(username);
	auto password = const_cast<SQLWCHAR*>(pw_wstr);
	const auto passlen = lstrlen(password);

	my_raw_secret.close();

	SQLRETURN sqlcode;

	// Allocate environment handle
	sqlcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &myEnvironment);

	// Set the ODBC version environment attribute
	if (SQLSucceed(sqlcode))
	{
		sqlcode = SQLSetEnvAttr(myEnvironment, SQL_ATTR_ODBC_VERSION, SQLPOINTER(SQL_OV_ODBC3), 0);

		// Allocate connection handle
		if (SQLSucceed(sqlcode))
		{
			sqlcode = SQLAllocHandle(SQL_HANDLE_DBC, myEnvironment, &myConnector);

			// Set login timeout to 5 seconds
			if (SQLSucceed(sqlcode))
			{
				constexpr std::ptrdiff_t timeout_second = 5;
				SQLSetConnectAttr(myConnector, SQL_LOGIN_TIMEOUT, SQLPOINTER(timeout_second), 0);

				// Connect to data source
				sqlcode = SQLConnect(myConnector, entry, SQL_NTS, username, userlen, password, passlen);

				if (SQLSucceed(sqlcode))
				{
					return true;
				}
				else
				{
					std::cout << "SQL 서버 로그인 실패!\n";
				}
			}
		}
	}

	return false;
}

bool DatabaseService::Disconnect()
{
	std::cout << "DB 관리 객체 정리 중...\n";

	if (NULL != myConnector)
	{
		isConnected = false;

		auto sqlcode = SQLDisconnect(myConnector);
		if (SQLSucceed(sqlcode))
		{
			SQLFreeHandle(SQL_HANDLE_DBC, myConnector);
			SQLFreeHandle(SQL_HANDLE_ENV, myEnvironment);
		}
		else
		{
			return false;
		}
	}

	return true;
}

shared_ptr<DatabaseQuery> DatabaseService::CreateQuery(std::wstring_view statement)
{
	auto result = make_shared<DatabaseQuery>(std::wstring{ statement });

	auto sqlcode = SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &result->myQuery);
		//CreateStatementAt(result.myQuery);

	if (SQLSucceed(sqlcode))
	{
		sqlcode = SQLPrepare(result->myQuery, (SQLWCHAR*) (statement.data()), SQL_NTS);
			//PrepareStatement(result.myQuery, statement);

		if (!SQLSucceed(sqlcode))
		{
			throw std::runtime_error("SQL Error!");
		}
	}
	else
	{
		throw std::runtime_error("SQL Error!");
	}

	return result;
}

SQLHSTMT DatabaseService::CreateStatement()
{
	SQLHSTMT hstmt{};

	SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &hstmt);

	return hstmt;
}

SQLRETURN DatabaseService::CreateStatementAt(SQLHSTMT& place)
{
	return SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &place);
}

SQLRETURN DatabaseService::PrepareStatement(const SQLHSTMT& statement, const std::wstring_view& query)
{
	return SQLPrepare(statement, (SQLWCHAR*) (query.data()), SQL_NTS);
}

const std::wstring& DatabaseService::GetStatement(std::wstring_view tag) const
{
	return myReadyStatement.at(tag);
}

DatabaseQuery& DatabaseService::GetQuery(std::wstring_view tag)
{
	return *(myQueries.at(tag));
}

const DatabaseQuery& DatabaseService::GetQuery(std::wstring_view tag) const
{
	return *(myQueries.at(tag));
}