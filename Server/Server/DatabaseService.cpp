#include "pch.hpp"
#include "DatabaseService.hpp"
#include "DatabaseQuery.hpp"

db::Service::Service()
	: myJobQueue(), JobBarrier()
	, myStatements()
	, myEnvironment(NULL), myConnector(NULL)
	, isConnected(false)
{
	myStatements.reserve(20);
}

db::Service::~Service()
{
	if (isConnected)
	{
		if (!myLastJob.expired())
		{
			myLastJob.lock()->Cancel();
		}

		Disconnect();
	}
}

bool db::Service::Awake()
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

				if (!SQLSucceed(sqlcode))
				{
					std::cout << "SQL 서버 로그인 실패!\n";
					return false;
				}
				else
				{
					return true;
				}
			}
		}
	}

	return false;
}

void db::Service::Start()
{
	auto prepared_statements = BuildPreparedStatements();

	for (const auto& [tag, statement] : prepared_statements)
	{
		RegisterStatement(tag, statement);
	};
}

constexpr std::vector<std::wstring_view> BuildDatabaseTags()
{
	return std::vector<std::wstring_view>
	{
		L"FIND_USER_NN",
		L"FIND_USERS_ALL",
		L"FIND_USER_EMAIL",
		L"FIND_USER_PW"
	};
}

constexpr std::vector<std::tuple<std::wstring_view, std::wstring_view>> BuildPreparedStatements()
{
	const auto tags = BuildDatabaseTags();

	std::vector<std::tuple<std::wstring_view, std::wstring_view>> result{};
	result.reserve(10);

	result.emplace_back(tags.at(0)
		, L"SELECT [ID], [NICKNAME] FROM [Users] WHERE [NICKNAME] = {}");

	return result;
}

bool db::Service::Disconnect()
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

db::Query& db::Service::PushJob(std::wstring_view&& statement)
{
	std::scoped_lock locken{ JobBarrier };

	return *myJobQueue.emplace(CreateQuery(std::forward<std::wstring_view>(statement)));
}

db::Query& db::Service::PushJob(shared_ptr<db::Query> query)
{
	std::scoped_lock locken{ JobBarrier };
	myJobQueue.push(query);

	return *query;
}

db::Query& db::Service::PushJobByTag(std::wstring_view&& tag, std::wformat_args&& args)
{
	std::scoped_lock locken{ JobBarrier };

	auto [statement, query] = GetStatement(std::forward<std::wstring_view>(tag));

	const auto formatted = std::vformat(statement, std::forward<std::wformat_args>(args));

	return PushJob(formatted);
}

db::Query& db::Service::PushJobByTag(std::wstring_view&& tag)
{
	std::scoped_lock locken{ JobBarrier };

	auto [statement, query] = GetStatement(std::forward<std::wstring_view>(tag));

	return PushJob(statement);
}

shared_ptr<db::Query> db::Service::PopJob()
{
	std::scoped_lock locken{ JobBarrier };

	auto result = myJobQueue.front();
	myJobQueue.pop();

	myLastJob = result;

	return result;
}

db::Query& db::Service::RegisterStatement(std::wstring_view tag, std::wstring_view statement)
{
	std::scoped_lock locken{ JobBarrier };

	myStatements.try_emplace(tag, statement);
	auto& tagged = myQueries[tag] = CreateQuery(statement);

	return *tagged;
}

shared_ptr<db::Query> db::Service::CreateQuery(std::wstring_view statement)
{
	auto result = make_shared<db::Query>(std::wstring{ statement });

	auto sqlcode = CreateStatementAt(result->myQuery);

	if (SQLSucceed(sqlcode))
	{
		sqlcode = PrepareStatement(result->myQuery, statement);

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

SQLHSTMT db::Service::CreateStatement()
{
	SQLHSTMT hstmt{};

	SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &hstmt);

	return hstmt;
}

SQLRETURN db::Service::CreateStatementAt(SQLHSTMT& place)
{
	return SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &place);
}

SQLRETURN db::Service::PrepareStatement(const SQLHSTMT& statement, const std::wstring_view& query)
{
	return SQLPrepare(statement, (SQLWCHAR*) (query.data()), SQL_NTS);
}

const std::pair<const std::wstring&, const shared_ptr<const db::Query>>
db::Service::GetStatement(std::wstring_view tag) const
{
	return std::make_pair(myStatements.at(tag), myQueries.at(tag));
}

const std::pair<std::wstring, shared_ptr<db::Query>>
db::Service::GetStatement(std::wstring_view tag)
{
	return std::make_pair(myStatements.at(tag), myQueries.at(tag));
}

bool db::Service::IsEmpty() const noexcept
{
	return myJobQueue.empty();
}

bool db::Service::IsPreparedEmpty() const noexcept
{
	return myQueries.empty();
}

db::Query& db::Service::GetQuery(std::wstring_view&& tag)
{
	return *(myQueries.at(std::forward<std::wstring_view>(tag)));
}

const db::Query& db::Service::GetQuery(std::wstring_view&& tag) const
{
	return *(myQueries.at(std::forward<std::wstring_view>(tag)));
}