#include "Engine.h"
#include "perft_tests.h"
#include <iostream>
#include <string>


int main(int argc, char* argv[])
{
	Engine engine = Engine();
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <path_to_fens_with_perft_results.txt> <first_index (optional)> <last_index (optional)>" << std::endl;
		return 1;
	}
	std::string path = argv[1];
	size_t first_index = 0;
	size_t last_index = SIZE_MAX;
	if (argc >= 3)
	{
		try
		{
			first_index = std::stoul(argv[2]);

		}
		catch (const std::exception& e)
		{
			std::cout << "Error parsing first index: " << e.what() << std::endl;
			return 1;
		}
	}
	if (argc >= 4)
	{
		try
		{
			last_index = std::stoul(argv[3]);

		}
		catch (const std::exception& e)
		{
			std::cout << "Error parsing last index: " << e.what() << std::endl;
			return 1;
		}
	}
	test_fens_with_perft(engine, path, first_index, last_index);



	return 0;
}
