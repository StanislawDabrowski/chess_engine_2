#pragma once



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
