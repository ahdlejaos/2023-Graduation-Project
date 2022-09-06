#include "pch.hpp"
#include "Framework.hpp"

Framework globalFramework{ srv::THREADS_COUNT };

int main()
{
	globalFramework.Awake(srv::SERVER_PORT_TCP);
	globalFramework.Start();
	globalFramework.Update();

	/*
	constexpr int test_list[]{ 1, 2, 3, 4, 5, 6, 7, 8 };

	constexpr auto TestEncrypt()
	{
		MD5 encryptor{};

		return encryptor("10");
	}

	std::cout << "index_view 테스트\n";

	std::vector test_vector{ 1, 2, 3, 4, 5, 6, 7, 8 };

	auto aa = test_vector.at(0);
	//auto test_a_view = std::views::all(test_vector);

	index_enumerator test_it{ test_vector, 6 };
	for (; test_it != test_vector.end(); test_it++)
	{
		const auto &valit = test_it.handle;
		const auto &valid = test_it.index;

		//std::cout << "it: " << (*valit) << ", id: " << valid << "\n";
		auto [val, ind] = *test_it;

		std::cout << "it: " << (*val) << ", id: " << ind << "\n";
	}

	std::cout << "MD5 테스트\n";

	const auto temp = TestEncrypt();
	std::cout << temp << "\n";
	*/

	// Release가 호출됨
	return 0;
}
