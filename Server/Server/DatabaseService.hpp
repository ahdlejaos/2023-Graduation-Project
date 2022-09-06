#pragma once
#include <mysql/jdbc.h>

class DatabaseService
{
public:
	DatabaseService(const std::string& db_server, const std::string& db_scheme);
	~DatabaseService();

	void Connect();
	void Disconnect();

	sql::PreparedStatement CreateArgStatement(const std::string& statement);
	sql::Statement CreateStatement(const std::string& statement);

	const std::string myServerAddress;
	const std::string myDatabase;

	const std::string superUsername = "iconerwokrs";
	const std::string superPassword = "00000000000";

	sql::Driver* myDriver;
	sql::Connection* myConnection;
	sql::Statement* myQuery;
	sql::PreparedStatement* myMutableQuery;
};
