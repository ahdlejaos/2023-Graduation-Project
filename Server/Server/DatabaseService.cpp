#include "pch.hpp"
#include "DatabaseService.hpp"

DatabaseService::DatabaseService()
{}

DatabaseService::~DatabaseService()
{}

void DatabaseService::Connect()
{
	try
	{
		std::string endpointUrl = myServerAddress;
		std::string accountName = superUsername;
		std::string accountKey = superPassword;

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
		catch (const Azure::Core::RequestFailedException& e)
		{
			std::cout << "Status Code: " << static_cast<int>(e.StatusCode)
				<< ", Reason Phrase: " << e.ReasonPhrase << std::endl;
			std::cout << e.what() << std::endl;
			
			return;
		}

		//return;
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
