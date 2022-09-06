#pragma once
#include <mysql/jdbc.h>

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	void Connect();
	void Disconnect();

	sql::PreparedStatement CreateArgStatement(const std::string_view statement);
	sql::Statement CreateStatement(const std::string_view statement);

	const std::string myServerAddress = "tcp://iconer-2023.database.windows.net:1433";
	const std::string myDatabase = "SkyRunner-MainServer";
	const std::string superUsername = "iconerworks@iconer-2023.database.windows.net";
	const std::string superUsername = "iconerwokrs";
	const std::string superPassword = "00000000000";

	sql::Driver* myDriver;
	sql::Connection* myConnection;
	sql::Statement* myQuery;
	sql::PreparedStatement* myMutableQuery;
};
