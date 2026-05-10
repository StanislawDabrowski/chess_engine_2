#include "unit_tests.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Board.h"
#include "MoveGenerator.h"
#include "Utils.h"
#include <sstream>
#include <vector>
#include <limits.h>
#ifdef __linux__
#include <unistd.h>
#else
static char dummy = (std::exit(1), 0)
#endif

bool terminate_on_failure = false;

std::string get_executable_file_directory_path()
{
#ifdef __linux__
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count == -1)
	{
		std::cerr << "Failed to get executable path" << std::endl;
		return "";
	}
	std::string path(result, count);
	size_t last_slash_pos = path.find_last_of('/');
	if (last_slash_pos == std::string::npos)
	{
		std::cerr << "Failed to parse executable path" << std::endl;
		return "";
	}
	return path.substr(0, last_slash_pos);
#endif
	
}

void run_all_tests(bool terminate_on_failure_arg)
{
	terminate_on_failure = terminate_on_failure_arg;
	MoveGeneratorTests::in_check_test();
	BoardTests::zobrist_hashing_test(get_executable_file_directory_path() + "/ethereal_fen_set.fen");
	BoardTests::repetition_detection_test(get_executable_file_directory_path() + "/draw_by_repetition_test_cases.txt");
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
	bool any_test_failed = false;
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
	if (!any_test_failed)
		std::cout << "MoveGeneratorTests::in_check_test passed" << std::endl;
}



namespace hash_tests
{
	void perft_with_hash_testing(Board* board, MoveGenerator* mg, uint8_t depth)
	{
		if (depth == 0)
			return;
		if (board->side_to_move == White)
		{
			mg->generate_pseudo_legal_moves<White>();
			mg->filter_pseudo_legal_moves<White>();
		}
		else
		{
			mg->generate_pseudo_legal_moves<Black>();
			mg->filter_pseudo_legal_moves<Black>();
		}
		uint64_t prev_hash, hash_after_move;
		for (size_t i = 0; i < board->positions_stack[board->current_position_idx].legal_moves_length; ++i)
		{
			prev_hash = board->positions_stack[board->current_position_idx].hash;
			board->make_move(board->positions_stack[board->current_position_idx].legal_moves[i]);
			hash_after_move = board->positions_stack[board->current_position_idx].hash;
			board->calculate_hash();
			if (hash_after_move != board->positions_stack[board->current_position_idx].hash)
			{
				board->unmake_move();//unmake to get the fen for the error message
				std::cout << "Hash after making a move did not match the calculated hash. move: " << Utils::move_to_string(board->positions_stack[board->current_position_idx].legal_moves[i]) << " in position: " << Utils::get_fen(board) << std::endl;
				if (terminate_on_failure)
				{
					exit(1);
				}
				board->make_move(board->positions_stack[board->current_position_idx].legal_moves[i]);//make the move again to continue the test
			}
			board->unmake_move();
			if (hash_after_move == prev_hash)
			{
				std::cout << "Hash did not change after move: " << Utils::move_to_string(board->positions_stack[board->current_position_idx].legal_moves[i]) << " in position: " << Utils::get_fen(board) << std::endl;
				if (terminate_on_failure)
				{
					exit(1);
				}
			}
			if (prev_hash != board->positions_stack[board->current_position_idx].hash)
			{
				std::cout << "Hash did not revert back after unmaking move: " << Utils::move_to_string(board->positions_stack[board->current_position_idx].legal_moves[i]) << " in position: " << Utils::get_fen(board) << std::endl;
				if (terminate_on_failure)
				{
					exit(1);
				}
			}
			board->make_move(board->positions_stack[board->current_position_idx].legal_moves[i]);
			//check if the hash is the same
			if (board->positions_stack[board->current_position_idx].hash != hash_after_move)
			{
				board->unmake_move();//unmake to get the fen for the error message
				std::cout << "Hash after unmaking a move and then making it again did not stay the same. move: " << Utils::move_to_string(board->positions_stack[board->current_position_idx].legal_moves[i]) << " in position: " << Utils::get_fen(board) << std::endl;
				if (terminate_on_failure)
				{
					exit(1);
				}
				board->make_move(board->positions_stack[board->current_position_idx].legal_moves[i]);//make the move again to continue the test
			}
			
			perft_with_hash_testing(board, mg, depth - 1);
			board->unmake_move();
		}

	}

	void test_zobrist_hashing_on_fen_with_perft(std::string fen, uint8_t depth)
	{
		Board board = Board();
		board.load_fen(fen);
		board.calculate_hash();
		MoveGenerator move_generator(&board);

		perft_with_hash_testing(&board, &move_generator, depth);
	}
}


void BoardTests::zobrist_hashing_test(std::string file_with_fens)
{
	constexpr uint8_t depth = 4;
	bool any_test_failed = false;
	std::ifstream file(file_with_fens);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_with_fens << std::endl;
		any_test_failed = true;
		return;
	}
	std::string line;
	int line_number = 0;
	while (std::getline(file, line))
	{
		hash_tests::test_zobrist_hashing_on_fen_with_perft(line, depth);
	}
	if (!any_test_failed)
		std::cout << "BoardTests::zobrist_hashing_test passed" << std::endl;
}

void BoardTests::repetition_detection_test(std::string file_with_fens_and_moves)
{
	std::ifstream file(file_with_fens_and_moves);
	bool any_test_failed = false;
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_with_fens_and_moves << std::endl;
		any_test_failed = true;
		return;
	}
	std::string line;
	int line_number = 0;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string fen;
		std::string temp;
		for (int i = 0; i < 6; ++i)
		{
			iss >> temp;
			fen += temp + (i < 5 ? " " : "");
		}
		std::vector<std::string> moves;
		std::string move_str;
		while (iss >> move_str)
		{
			moves.push_back(move_str);
		}
		Board board = Board();
		board.load_fen(fen);
		for (int i = 0; i < moves.size(); ++i)
		{
			Move move = Utils::string_to_move(&board, moves[i]);
			board.make_move(move);
			if (i != moves.size()-1 && board.positions_stack[board.current_position_idx].draw_by_repetition)
			{
				std::cerr << "Incorrectly detect repetition in line " << line_number << " root fen: \"" << fen << "\" after move " << moves[i] << " (" << i + 1 << ". move)" << std::endl;
				any_test_failed = true;
				if (terminate_on_failure)
				{
					exit(1);
				}
				break;
			}
			else if (i==moves.size()-1 && !board.positions_stack[board.current_position_idx].draw_by_repetition)
			{
				std::cerr << "Failed to detected repetition in line " << line_number << " root fen: " << fen << " after move " << moves[i] << " (" << i + 1 << ". move)" << std::endl;
				any_test_failed = true;
				if (terminate_on_failure)
				{
					exit(1);
				}
				break;
			}
		}
		line_number++;
	}
	if (!any_test_failed)
		std::cout << "BoardTests::repetition_detection_test passed" << std::endl;
}
