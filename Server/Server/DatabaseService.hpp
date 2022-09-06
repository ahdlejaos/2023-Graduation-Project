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

	const std::string myServerAddress;
	const std::string myDatabase;

	const std::string superUsername = "iconerwokrs";
	const std::string superPassword = "00000000000";

	sql::Driver* myDriver;
	sql::Connection* myConnection;
	sql::Statement* myQuery;
	sql::PreparedStatement* myMutableQuery;
};
