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

	bool Connect();
	bool Disconnect();

	sql::PreparedStatement CreateArgStatement(const std::string_view statement);
	sql::Statement CreateStatement(const std::string_view statement);
};
