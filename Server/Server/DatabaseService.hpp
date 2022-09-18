#pragma once
#include <sql.h>
#include <sqlext.h>

class DatabaseQuery;

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Awake();
	bool Disconnect();

	std::optional<DatabaseQuery> CreateQuery(const std::wstring_view& query);

	template<typename... Ty>
	std::optional<DatabaseQuery> Execute(std::tuple<Ty...> args)&;
	template<typename... Ty>
	std::optional<DatabaseQuery> Execute(std::tuple<Ty...> args) &&;

	SQLHENV myEnvironment;
	SQLHDBC myConnector;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";
};

class DatabaseQuery
{
public:
	constexpr DatabaseQuery()
		: myQuery(NULL)
	{}

	constexpr DatabaseQuery(const SQLHSTMT query)
		: myQuery(query)
	{}

	~DatabaseQuery()
	{
		if (NULL != myQuery)
		{
			Destroy();
			myQuery = NULL;
		}
	}

	SQLRETURN Cancel()
	{
		return SQLCancel(myQuery);
	}

	SQLRETURN Destroy()
	{
		return SQLFreeHandle(SQL_HANDLE_STMT, myQuery);
	}

	SQLHSTMT myQuery;
};

extern "C" let bool SQLSucceed(const SQLRETURN code) noexcept
{
	return (SQL_SUCCEEDED(code));
}

extern "C" let bool SQLHasParameters(const SQLRETURN code) noexcept
{
	return (code == SQL_PARAM_DATA_AVAILABLE);
}

extern "C" let bool SQLFailed(const SQLRETURN code) noexcept
{
	return (code == SQL_ERROR);
}

/* Allocate statement handle:

SQLHSTMT hstmt = 0;

constexpr int NAME_LEN = 30, PHONE_LEN = 30;
SQLCHAR szName[NAME_LEN]{}, szPhone[PHONE_LEN]{}, sCustID[NAME_LEN]{};
SQLLEN cbName = 0, cbCustID = 0, cbPhone = 0;

sqlcode = SQLAllocHandle(SQL_HANDLE_STMT, myConnector, &hstmt);
sqlcode = SQLExecDirect(hstmt, (SQLWCHAR*) L"SELECT CustomerID, ContactName, Phone FROM CUSTOMERS ORDER BY 2, 1, 3", SQL_NTS);

if (SQLSucceed(sqlcode))
{
	// Bind columns 1, 2, and 3
	sqlcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, sCustID, 100, &cbCustID);
	sqlcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);
	sqlcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, szPhone, PHONE_LEN, &cbPhone);

	// Fetch and print each row of data. On an error, display a message and exit.

	for (int i = 0; ; i++)
	{
		sqlcode = SQLFetch(hstmt);

		if (SQLFailed(sqlcode) || sqlcode == SQL_SUCCESS_WITH_INFO)
		{
			//show_error();
		}
		else if (SQLSucceed(sqlcode))
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
if (SQLSucceed(sqlcode))
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}
*/

/* PowerShell:

$Env:AZURE_CLIENT_ID="generated-app-ID"
$Env:AZURE_CLIENT_SECRET="random-password"
$Env:AZURE_TENANT_ID="tenant-ID"
*/

/* Azure Key Vault - KeyClient:
https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-security-keyvault-keys/4.2.0/index.html

// Create a new key client using the default credential from Azure Identity using environment variables previously set,
// including AZURE_CLIENT_ID, AZURE_CLIENT_SECRET, and AZURE_TENANT_ID.
auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
KeyClient client("AZURE_KEYVAULT_URL", credential);

// Create a new key using the key client.
client.CreateKey("key-name", KeyVaultKeyType::Rsa);

// Retrieve a key using the key client.
key = client.GetKey("key-name");

========================================================================

// Create a new cryptography client using the default credential from Azure Identity using environment variables previously set,
// including AZURE_CLIENT_ID, AZURE_CLIENT_SECRET, and AZURE_TENANT_ID.
auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
CryptographyClient cryptoClient(key.Id, credential);

*/

/* Ready:

#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/storage_common.hpp>

auto tenantId = std::getenv("AZURE_TENANT_ID");
auto clientId = std::getenv("AZURE_CLIENT_ID");
auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
const std::string leaseID = "leaseID";
const std::string smokeUrl = "https://blob.com";
// Creating an attestation service instance requires contacting the attestation service (to
// retrieve validation collateral). Use the West US Shared client (which should always be
// available) as an anonymous service instance.
const std::string attestationUrl = "https://sharedwus.wus.attest.azure.net";

auto credential
	= std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

*/

/* Azure

*/
