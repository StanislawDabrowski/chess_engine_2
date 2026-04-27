#include "unit_tests.h"
#include <string>
#include <iostream>
#include "Board.h"
#include "MoveGenerator.h"

bool terminate_on_failure = false;
bool any_test_failed = false;

void run_all_tests(bool terminate_on_failure_arg)
{
	terminate_on_failure = terminate_on_failure_arg;
	MoveGeneratorTests::in_check_test();
	if (!any_test_failed)
		std::cout << "All tests passed" << std::endl;
}

void MoveGeneratorTests::in_check_test()
{
	TestCase<std::string, bool> test_cases[] = {
		{"8/1kp5/8/1n6/8/5B2/8/4K3 b - - 0 1", true},
		{"8/1k6/2p5/1n6/8/5B2/8/4K3 b - - 0 1", false},
		{"8/1k6/2p5/8/8/5B2/2n5/4K3 w - - 0 1", true},
		{"8/1k6/2p5/8/8/5B2/2n5/3K4 w - - 0 1", false},
		{"8/1k6/2pN4/8/8/5B2/2n5/3K4 b - - 0 1", true},
		{"8/8/1kpN4/8/8/5B2/2n5/3K4 b - - 0 1", false},
		{"8/8/1kpN4/8/8/1b1n1B2/8/3K4 w - - 0 1", true},
		{"8/8/1kpN4/8/8/1b1n1B2/3K4/8 w - - 0 1", false},
		{"8/8/1kpN4/2P5/8/1b1n1B2/3K4/8 b - - 0 1", true},
		{"8/8/k1pN4/2P5/8/1b1n1B2/3K4/8 b - - 0 1", false},
		{"8/8/k1pN4/2P5/8/1bpn1B2/3K4/8 w - - 0 1", true},
		{"8/8/k1pN4/2P5/8/1bpn1B2/4K3/8 w - - 0 1", false},
		{"8/8/k1pN4/2P5/2Q5/1bpn1B2/4K3/8 b - - 0 1", true},
		{"8/k7/2pN4/2P5/2Q5/1bpn1B2/4K3/8 b - - 0 1", false},
		{"8/k7/2pN4/2P1q3/2Q5/1bpn1B2/4K3/8 w - - 0 1", true},
		{"8/k7/2pN4/2P1q3/2Q5/1bpn1B2/8/5K2 w - - 0 1", false},
		{"8/k7/2pN4/2P1q3/2Q5/Rbpn1B2/8/5K2 b - - 0 1", true},
		{"1k6/8/2pN4/2P1q3/2Q5/Rbpn1B2/8/5K2 b - - 0 1", false},
		{"1k6/8/2pN4/2P1q3/2Q5/Rbpn1B2/8/2r2K2 w - - 0 1", true},
		{"1k6/8/2pN4/2P1q3/2Q5/Rbpn1B2/6K1/2r5 w - - 0 1", false},
		{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", false},
	};
	Board board = Board();
	MoveGenerator move_generator(&board);
	move_generator.initialize_attack_tables();
	bool result;
	for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
	{
		board.load_fen(test_cases[i].input);
		if (board.side_to_move == White)
			result = move_generator.in_check<White>();
		else
			result = move_generator.in_check<Black>();
		if (result != test_cases[i].expected)
		{
			std::cout << "in_check: test case " << i << " failed: input = " << test_cases[i].input << ", expected = " << test_cases[i].expected << ", got = " << result << std::endl;
			any_test_failed = true;
			if (terminate_on_failure)
				exit(1);
		}
	}
}
