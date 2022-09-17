#include "pch.hpp"
#include "DatabaseService.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

DatabaseService::DatabaseService()
	: myEnvironment(NULL), myConnector(NULL)
{}

DatabaseService::~DatabaseService()
{
	Disconnect();
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
		sqlcode = SQLSetEnvAttr(myEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*) SQL_OV_ODBC3, 0);

		// Allocate connection handle
		if (SQLSucceed(sqlcode))
		{
			sqlcode = SQLAllocHandle(SQL_HANDLE_DBC, myEnvironment, &myConnector);

			// Set login timeout to 5 seconds
			if (SQLSucceed(sqlcode))
			{
				constexpr int timeout_second = 5;
				SQLSetConnectAttr(myConnector, SQL_LOGIN_TIMEOUT, SQLPOINTER(timeout_second), 0);

				// Connect to data source
				sqlcode = SQLConnect(myConnector, entry, SQL_NTS, username, userlen, password, passlen);

				if (SQLSucceed(sqlcode))
				{
					return true;
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

std::optional<DatabaseQuery> DatabaseService::CreateQuery()
{
	std::optional<DatabaseQuery> result{};
	SQLHSTMT hstmt = 0;

	constexpr int NAME_LEN = 30, PHONE_LEN = 30;
	SQLCHAR szName[NAME_LEN]{}, szPhone[PHONE_LEN]{}, sCustID[NAME_LEN]{};
	SQLLEN cbName = 0, cbCustID = 0, cbPhone = 0;

	auto sqlcode = SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &hstmt);
	sqlcode = SQLExecDirect(hstmt, (SQLWCHAR*) L"SELECT CustomerID, ContactName, Phone FROM CUSTOMERS ORDER BY 2, 1, 3", SQL_NTS);

	if (SQLSucceed(sqlcode))
	{
		// Bind columns 1, 2, and 3
		sqlcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, sCustID, 100, &cbCustID);
		sqlcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);
		sqlcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, szPhone, PHONE_LEN, &cbPhone);

		// Fetch and print each row of data. On an error, display a message and exit.

		for (int i = 0; ; i++)
		{
			sqlcode = SQLFetch(hstmt);

			if (SQLFailed(sqlcode) || sqlcode == SQL_SUCCESS_WITH_INFO)
			{
				//show_error();
			}
			else if (SQLSucceed(sqlcode))
			{
				printf("%d: %s %s %sn", i + 1, sCustID, szName, szPhone);

				result = hstmt;
			}
			else
			{
				break;
			}
		}
	}

	return result;
}
