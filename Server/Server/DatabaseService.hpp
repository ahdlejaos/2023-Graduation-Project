#pragma once
#include <sql.h>
#include <sqlext.h>

#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/storage_common.hpp>

class DatabaseService
{
public:
	DatabaseService();
	~DatabaseService();

	bool Connect();
	bool Disconnect();

	SQLHENV myEnvironment;
	SQLHDBC myConnector;

	const std::wstring myEntry = L"2023-Graduation-Project";
	const Filepath mySecrets = ".//data//Secrets.json";
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
