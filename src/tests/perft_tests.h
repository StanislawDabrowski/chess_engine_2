#pragma once
#include "Engine.h"
#include <string>




std::pair<bool, uint64_t> test_perft(Engine& engine, std::string fen, uint8_t depth, uint64_t expected);
void test_fens_with_perft(Engine& engine, std::string path, size_t first_position_index, size_t last_position_index);// path should be a text file with lines in the format: fen;depth;expected_perft_count
