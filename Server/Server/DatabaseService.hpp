#pragma once
#include "MySQL.hpp"

struct DatabaseJob
{
	shared_ptr<DatabaseQuery> query;
};

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Awake();
	bool Disconnect();

	DatabaseQuery& RegisterQuery(std::wstring_view tag, std::wstring_view statement);

	bool CreateQuery(std::wstring_view statement) const;
	shared_ptr<DatabaseQuery> CreateQuery(std::wstring_view statement);

	const std::wstring& GetStatement(std::wstring_view tag) const;
	DatabaseQuery& GetQuery(std::wstring_view tag);
	const DatabaseQuery& GetQuery(std::wstring_view tag) const;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";

	SQLHENV myEnvironment;
	SQLHDBC myConnector;
	bool isConnected;

private:
	SQLHSTMT CreateStatement();
	SQLRETURN CreateStatementAt(SQLHSTMT& place);
	SQLRETURN PrepareStatement(const SQLHSTMT& statement, const std::wstring_view& query);

	std::vector<DatabaseJob> myQueue;
	std::unordered_map<std::wstring_view, std::wstring> myReadyStatement;
	std::unordered_map<std::wstring_view, shared_ptr<DatabaseQuery>> myQueries;
};

