#pragma once

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

template<typename... Ty>
inline auto FormatQuery(std::wstring query, Ty... args)
{
	//query.replace(query.begin(), query.end(), L"{}", args);

	//std::replace(query.begin(), query.end(), L"{}");
	return std::format(query, args...);
}
