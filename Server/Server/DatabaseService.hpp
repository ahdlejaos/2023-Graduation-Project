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

DatabaseService::DatabaseService(const std::string_view db_server, const std::string_view db_scheme)
	: myServerAddress(db_server), myDatabase(db_scheme)
	, myDriver(nullptr), myConnection(nullptr), myQuery(nullptr), myMutableQuery(nullptr)
{
}

inline DatabaseService::~DatabaseService()
{
	if (myDriver)
	{
		myDriver->threadEnd();
	}
	if (myConnection)
	{
		if (!myConnection->isClosed())
		{
			myConnection->close();
		}
	}
}

inline void DatabaseService::Connect()
{
	try
	{
		myDriver = ::get_driver_instance();
		myConnection = myDriver->connect(myServerAddress, superUsername, superPassword);
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "SQL 예외 발생" << e.what() << "\n";
		return;
	}
	catch (std::exception& e)
	{
		std::cerr << "예외 발생: " << e.what() << "\n";
		return;
	}

	myConnection->setSchema(myDatabase);
}
