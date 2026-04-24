#include "perft_tests.h"
#include "Engine.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool test_perft(Engine& engine, std::string fen, uint8_t depth, uint64_t expected)
{
	engine.board.load_fen(fen);
	uint64_t result = engine.perft(depth);
	if (result != expected)
	{
		return false;
	}
	return true;
}


void test_fens_with_perft(Engine& engine, std::string path, size_t first_position_index, size_t last_position_index)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return;
	}
	std::string line;
	int line_number = -1;
	int total_tests = 0;
	int failed_tests = 0;
	while (std::getline(file, line))
	{
		line_number++;
		if (line_number < first_position_index) continue;
		if (line_number > last_position_index) break;
		if (line.empty()) continue;
		std::cout << "\rTesting line " << line_number << std::flush;
		std::istringstream iss(line);
		std::string fen;
		uint8_t depth;
		uint64_t expected;

		std::getline(iss, fen, ';');
		std::string depth_str, expected_str;
		std::getline(iss, depth_str, ';');
		std::getline(iss, expected_str, ';');
		try
		{
			depth = std::stoi(depth_str);
			expected = std::stoull(expected_str);
		}
		catch (const std::exception& e)
		{
			std::cout << "Error parsing line " << line_number << ": " << e.what() << std::endl;
			continue;
		}

		if (!test_perft(engine, fen, depth, expected))
		{
			failed_tests++;
			std::cout << "Test failed for line " << line_number << ": " << line << " got: " << engine.perft(depth) << " expected: " << expected << std::endl;
		}
		total_tests++;
		
		
	}

	std::cout << "\nTotal tests: " << total_tests << " Failed tests: " << failed_tests << std::endl;
	//print percentage of tests passed
	std::cout << "Passed: " << ((total_tests - failed_tests) * 100.0 / total_tests) << "%" << std::endl;

	file.close();
}
