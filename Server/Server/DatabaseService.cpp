#include "pch.hpp"
#include "DatabaseService.hpp"

DatabaseService::DatabaseService(const std::string_view db_server, const std::string_view db_scheme)
	: myServerAddress(db_server), myDatabase(db_scheme)
	, myDriver(nullptr), myConnection(nullptr), myQuery(nullptr), myMutableQuery(nullptr)
{}

DatabaseService::~DatabaseService()
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

void DatabaseService::Connect()
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
