#pragma once
#include "MySQL.hpp"

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Awake();
	bool Disconnect();

	std::optional<DatabaseQuery> CreateQuery(std::wstring_view query);

	SQLHENV myEnvironment;
	SQLHDBC myConnector;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";

	friend class Framework;

private:
	SQLHSTMT CreateStatement();
	SQLRETURN CreateStatementAt(SQLHSTMT& place);
	SQLRETURN PrepareStatement(const SQLHSTMT& statement, const std::wstring_view& query);
};
