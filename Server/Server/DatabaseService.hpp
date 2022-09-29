#pragma once
#include "MySQL.hpp"
#include "BasicDatabaseJob.hpp"
#include "DatabaseJob.hpp"

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Awake();
	void Start();
	bool Disconnect();

	DatabaseQuery& PushJob(std::wstring_view&& statement);
	DatabaseQuery& PushJob(shared_ptr<DatabaseQuery> query);
	DatabaseQuery& PushJobByTag(std::wstring_view&& tag, std::wformat_args&& args);
	DatabaseQuery& PushJobByTag(std::wstring_view&& tag);
	shared_ptr<DatabaseQuery> PopJob();

	DatabaseQuery& RegisterQuery(std::wstring_view tag, std::wstring_view statement);
	const std::pair<const std::wstring&, const shared_ptr<const DatabaseQuery>> GetStatement(std::wstring_view tag) const;
	const std::pair<std::wstring, shared_ptr<DatabaseQuery>> GetStatement(std::wstring_view tag);

	DatabaseQuery& GetQuery(std::wstring_view&& tag);
	const DatabaseQuery& GetQuery(std::wstring_view&& tag) const;

	bool IsEmpty() const noexcept;
	bool IsPreparedEmpty() const noexcept;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";

	SQLHENV myEnvironment;
	SQLHDBC myConnector;
	bool isConnected;

private:
	SQLHSTMT CreateStatement();
	SQLRETURN CreateStatementAt(SQLHSTMT& place);
	SQLRETURN PrepareStatement(const SQLHSTMT& statement, const std::wstring_view& query);
	shared_ptr<DatabaseQuery> CreateQuery(std::wstring_view statement);

	std::array<DatabaseJob, 100> myJobPool;

	std::queue<shared_ptr<DatabaseQuery>> myJobQueue;
	std::mutex JobBarrier;
	weak_ptr<DatabaseQuery> myLastJob;

	std::unordered_map<std::wstring_view, std::wstring> myStatements;
	std::unordered_map<std::wstring_view, shared_ptr<DatabaseQuery>> myQueries;
};

class JobUserFindByID : BasicDatabaseJob<JobUserFindByID>
{
public:
	PID target_id;
};

class JobUserFindByNickname : BasicDatabaseJob<JobUserFindByNickname>
{
public:
	wchar_t nickname[20];
};

class JobUserFindByNicknameIncluded : BasicDatabaseJob<JobUserFindByNicknameIncluded>
{
public:
	wchar_t nickname[20];
};
