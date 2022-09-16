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
