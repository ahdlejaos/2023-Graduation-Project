#pragma once
#include "MySQL.hpp"
#include "DatabaseJob.hpp"

namespace db
{
	class Service
	{
	public:
		Service();
		~Service();

		bool Awake();
		void Start();
		bool Disconnect();

		db::Query& PushJob(std::wstring_view&& statement);
		db::Query& PushJob(shared_ptr<db::Query> query);
		db::Query& PushJobByTag(std::wstring_view&& tag, std::wformat_args&& args);
		db::Query& PushJobByTag(std::wstring_view&& tag);
		shared_ptr<db::Query> PopJob();

		db::Query& RegisterStatement(std::wstring_view tag, std::wstring_view statement);

		const std::pair<const std::wstring&, const shared_ptr<const db::Query>> GetStatement(std::wstring_view tag) const;
		const std::pair<std::wstring, shared_ptr<db::Query>> GetStatement(std::wstring_view tag);

		db::Query& GetQuery(std::wstring_view&& tag);
		const db::Query& GetQuery(std::wstring_view&& tag) const;

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
		shared_ptr<db::Query> CreateQuery(std::wstring_view statement);

		std::array<Job, 100> myJobPool;

		std::queue<shared_ptr<db::Query>> myJobQueue;
		std::mutex JobBarrier;
		weak_ptr<db::Query> myLastJob;

		std::unordered_map<std::wstring_view, std::wstring> myStatements;
		std::unordered_map<std::wstring_view, shared_ptr<db::Query>> myQueries;
	};
}

constexpr std::vector<std::wstring_view> BuildDatabaseTags();

constexpr std::vector<std::tuple<std::wstring_view, std::wstring_view>> BuildPreparedStatements();

namespace db
{
	class JobUserFindByID;

	class JobUserFindByID : BasicJob<JobUserFindByID>
	{
	public:
		PID target_id;
	};

	class JobUserFindByNickname : BasicJob<JobUserFindByNickname>
	{
	public:
		wchar_t nickname[20];
	};

	class JobUserFindByNicknameIncluded : BasicJob<JobUserFindByNicknameIncluded>
	{
	public:
		wchar_t nickname[20];
	};
}
