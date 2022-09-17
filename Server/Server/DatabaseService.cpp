#include "pch.hpp"
#include "DatabaseService.hpp"

DatabaseService::DatabaseService()
{}

DatabaseService::~DatabaseService()
{}

void DatabaseService::Connect()
{
	std::string endpointUrl = myServerAddress;
	std::string accountName = superUsername;
	std::string accountKey = superPassword;

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	const int NAME_LEN = 30, PHONE_LEN = 30;
	SQLCHAR szName[NAME_LEN]{}, szPhone[PHONE_LEN]{}, sCustID[NAME_LEN]{};
	SQLLEN cbName = 0, cbCustID = 0, cbPhone = 0;

	// Allocate environment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	// Set the ODBC version environment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*) SQL_OV_ODBC3, 0);

		// Allocate connection handle
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER) 5, 0);

				// Connect to data source
				retcode = SQLConnect(hdbc, (SQLCHAR*) "SQLCMD", SQL_NTS, (SQLCHAR*) "Test1", 5, (SQLCHAR*) "Password1", 9);
				// Allocate statement handle

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
					retcode = SQLExecDirect(hstmt, (SQLCHAR*) "SELECT CustomerID, ContactName, Phone FROM CUSTOMERS ORDER BY 2, 1, 3", SQL_NTS);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						// Bind columns 1, 2, and 3
						retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, sCustID, 100, &cbCustID);
						retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);
						retcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, szPhone, PHONE_LEN, &cbPhone);

						// Fetch and print each row of data. On an error, display a message and exit.

						for (int i = 0; ; i++)
						{
							retcode = SQLFetch(hstmt);

							if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
							{
								show_error();
							}
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
							{
								printf("%d: %s %s %sn", i + 1, sCustID, szName, szPhone);
							}
							else
							{
								break;
							}
						}
					}

					// Process data
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}

	using namespace Azure::Core;
	using namespace Azure::Storage;
	using namespace Azure::Storage::Blobs;

	try
	{
		auto credential_key = std::make_shared<StorageSharedKeyCredential>(accountName, accountKey);

		auto blockBlobClient = BlockBlobClient(endpointUrl, credential_key);

		// Create some data to upload into the blob.
		std::vector<uint8_t> data = { 1, 2, 3, 4 };
		Azure::Core::IO::MemoryBodyStream stream(data);

		Azure::Response<Models::UploadBlockBlobResult> response = blockBlobClient.Upload(stream);

		Models::UploadBlockBlobResult model = response.Value;
		std::cout << "Last modified date of uploaded blob: " << model.LastModified.ToString()
			<< std::endl;
	}
	catch (Azure::Core::OperationCancelledException& ce)
	{
		std::cerr << "연결 실패!\n오류 내용: " << ce.what();
	}
	catch (Azure::Core::RequestFailedException& ce)
	{
		std::cerr << "연결 실패!\n오류 내용: " << ce.what();
	}
}

void DatabaseService::Disconnect()
{}
