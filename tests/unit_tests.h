#pragma once
#include <string>



template<typename Input, typename Output>
struct TestCase
{
	Input input;
	Output expected;
};


void run_all_tests(bool terminate_on_failure_arg=false);

namespace MoveGeneratorTests
{
	void in_check_test();
}
namespace BoardTests
{
	void zobrist_hashing_test(std::string file_with_fens);
}
