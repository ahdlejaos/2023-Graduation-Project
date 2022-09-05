#pragma once

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
