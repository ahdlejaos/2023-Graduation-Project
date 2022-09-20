#pragma once
#include <sql.h>
#include <sqlext.h>

template<typename ...RetTy>
class DatabaseQuery;

extern "C" let bool SQLSucceed(const SQLRETURN code) noexcept;
extern "C" let bool SQLSucceedWithInfo(const SQLRETURN code) noexcept;
extern "C" let bool SQLHasParameters(const SQLRETURN code) noexcept;
extern "C" let bool SQLFetchEnded(const SQLRETURN code) noexcept;
extern "C" let bool SQLFailed(const SQLRETURN code) noexcept;
template<typename Ty> constexpr int DeductSQLType();

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Awake();
	bool Disconnect();

	template<typename ...RetTy>
	std::optional<DatabaseQuery<RetTy...>> CreateQuery(const std::wstring_view& query) const;

	SQLHENV myEnvironment;
	SQLHDBC myConnector;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";
};

template<typename ...RetTy>
class DatabaseQuery
{
public:
	constexpr DatabaseQuery()
		: myStatement()
		, myQuery(NULL), myData()
	{}

	constexpr DatabaseQuery(const SQLHSTMT query)
		: myStatement()
		, myQuery(query), myData()
	{}

	constexpr DatabaseQuery(const std::wstring_view& statement, const SQLHSTMT query)
		: myStatement(statement)
		, myQuery(query), myData()
	{}

	~DatabaseQuery()
	{
		if (NULL != myQuery)
		{
			Destroy();
			myQuery = NULL;
		}
	}

	bool Execute()
	{
		if (NULL != myQuery)
		{
			auto sqlcode = SQLExecute(myQuery);

			if (SQLSucceed(sqlcode))
			{
				return true;
			}
		}

		return false;
	}

	template<typename Ty>
	SQLRETURN Bind(std::size_t column, SQLSMALLINT sql_type, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, sql_type, place, length, result_length);
	}

	template<typename Ty>
	SQLRETURN Bind(std::size_t column, Ty* place, SQLLEN length, SQLLEN* result_length)
	{
		return SQLBindCol(myQuery, column, DeductSQLType<Ty>(), place, length, result_length);
	}

	SQLRETURN FetchOnce()
	{
		return SQLFetch(myQuery);
	}

	bool Fetch()
	{
		SQLRETURN sqlcode{};

		do
		{
			sqlcode = FetchOnce();

			if (SQLFailed(sqlcode))
			{
				break;
			}
			else if (SQLSucceedWithInfo(sqlcode))
			{

				//return true;
			}
			else if (SQLSucceed(sqlcode))
			{

				//return true;
			}
			else if (SQLFetchEnded(sqlcode))
			{
				//result = SQL_NO_DATA_FOUND;
				break;
			}
			else
			{
				break;
			}
		}
		while (SQLFetchEnded(sqlcode));
	}

	void ClearFetches()
	{
		myCursor = 0;
	}

	SQLRETURN Cancel()
	{
		return SQLCancel(myQuery);
	}

	SQLRETURN Destroy()
	{
		return SQLFreeHandle(SQL_HANDLE_STMT, myQuery);
	}

	std::wstring myStatement;
	SQLHSTMT myQuery;
	std::tuple<RetTy...> myData;
	int myCursor = 0;
};

template<typename Ty, std::size_t Size = 0>
struct QueryBinder : QueryBinder<std::remove_cv_t<Ty>, Size>
{};

template<typename Ty, std::size_t Size>
struct QueryBinder<Ty[Size], Size>
{
	Ty myData[Size]{};
};

template<typename Ty>
struct QueryBinder<Ty&, 0>
{
	Ty myData{};
};

template<typename Ty>
struct QueryBinder<Ty&&, 0>
{
	Ty myData{};
};

template<typename FirstTy, typename ...RestTy>
std::tuple<FirstTy, RestTy...>
FetchQ(const SQLHSTMT query, FirstTy&& first, RestTy&&... args)
{
	constexpr auto args_count = sizeof...(RestTy) + 1;
	std::tuple<FirstTy, RestTy...> result{};

	SQLRETURN sqlcode{};

	for (std::size_t i = 0; i < args_count; i++)
	{
		auto& place = std::get<i>(result);

		using PlaceType = decltype(place);
		
		int sql_type = DeductSQLType<PlaceType>();

		Bind(query, i, sql_type, &place, 20, nullptr);
	}

	do
	{
		result = SQLFetch(query);

		if (SQLFailed(sqlcode))
		{
			break;
		}
		else if (SQLSucceedWithInfo(sqlcode))
		{

			//return true;
		}
		else if (SQLSucceed(sqlcode))
		{

			//return true;
		}
		else if (SQLFetchEnded(sqlcode))
		{
			//result = SQL_NO_DATA_FOUND;
			break;
		}
		else
		{
			break;
		}
	}
	while (SQLFetchEnded(sqlcode));

	return result;
}

extern "C" let bool SQLSucceed(const SQLRETURN code) noexcept
{
	return (SQL_SUCCEEDED(code));
}

extern "C" let bool SQLSucceedWithInfo(const SQLRETURN code) noexcept
{
	return (code == SQL_SUCCESS_WITH_INFO);
}

extern "C" let bool SQLFetchEnded(const SQLRETURN code) noexcept
{
	return (code == SQL_NO_DATA_FOUND);
}

extern "C" let bool SQLHasParameters(const SQLRETURN code) noexcept
{
	return (code == SQL_PARAM_DATA_AVAILABLE);
}

extern "C" let bool SQLFailed(const SQLRETURN code) noexcept
{
	return (code == SQL_ERROR);
}

template<typename Ty>
constexpr int DeductSQLType()
{
	using PureTy = std::remove_cvref_t<Ty>;
	using NotArray = std::remove_all_extents_t<PureTy>;

	if constexpr (std::is_array_v<PureTy>)
	{
		if constexpr (std::is_base_of_v<char, NotArray>)
		{
			return SQL_C_CHAR;
		}
	}
	else if constexpr (std::is_floating_point_v<PureTy>)
	{
		if constexpr (std::is_same_v<PureTy, double>)
		{
			return SQL_C_DOUBLE;
		}
		else
		{
			return SQL_C_FLOAT;
		}
		return SQL_C_LONG;
	}
	else
	{
		if constexpr (std::is_same_v<PureTy, short>)
		{
			return SQL_C_SHORT;
		}
		else if constexpr (std::is_same_v<PureTy, unsigned short>)
		{
			return SQL_C_USHORT;
		}
		else if constexpr (std::is_unsigned_v<PureTy>)
		{
			return SQL_C_ULONG;
		}
		else
		{
			return SQL_C_LONG;
		}
	}

	return 0;
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
