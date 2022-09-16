#pragma once
#include "azure/core.hpp"

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	void Connect();
	void Disconnect();

	const std::string myServerAddress = "tcp://iconer-2023.database.windows.net:1433";

	const std::string myDatabase = "SkyRunner-MainServer";
	const std::string superUsername = "iconerworks@iconer-2023.database.windows.net";
	const std::string superPassword = "";
};
