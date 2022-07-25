#include "pch.hpp"
#include "Framework.hpp"

Framework globalFramework{};

constexpr int test_list[]{ 1, 2, 3, 4, 5, 6, 7, 8 };

int main()
{
	std::cout << "index_view 테스트\n";

	std::vector test_vector{ 1, 2, 3, 4, 5, 6, 7, 8 };

	auto aa = test_vector.at(0);
	//auto test_a_view = std::views::all(test_vector);

	index_view_enumerator test_it{ test_vector.begin() };
	for (; test_it != test_vector.end(); test_it++)
	{
		const auto &valit = test_it.handle;
		const auto &valid = test_it.index;

		std::cout << "it: " << (*valit) << ", id: " << valid << "\n";
		//auto [val, ind] = *it;

	}

	globalFramework.Awake(srv::THREADS_COUNT, srv::SERVER_PORT_TCP);
	globalFramework.Start();
	globalFramework.Update();
	globalFramework.Release();
}
