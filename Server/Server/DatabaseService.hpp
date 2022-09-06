#pragma once
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

class DatabaseService
{
public:
	DatabaseService(const std::string_view db_server, const std::string_view db_scheme);
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
