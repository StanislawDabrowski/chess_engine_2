#pragma once
#include "Board.h"
#include "MoveGenerator.h"
#include "StaticEval.h"
#include <type_traits>


class Engine
{
public:
	Board board;
	MoveGenerator mg;
	StaticEval se;
	uint64_t nodes_searched;

	static constexpr int16_t MAX_EVAL = 32767;
	static constexpr int16_t MIN_EVAL = -32767;//needs to be -32767 so -MIN_EVAL is MAX_EVAL, not itself, which due to integer overflow would probably (it's UB), be the case

	Engine();
	Engine(const Engine&);
	Engine& operator=(const Engine&);
	template<Color color>
	uint64_t perft(uint8_t depth);
	template<Color color, bool root = false, bool count_searched_nodes = false>//if root, return a pair of the best move and evaluation, otherwise return only the evaluation. Made that way to overheadlessly have only 1 definition of the search function for both root and non-root calls.
	std::conditional_t<root, std::pair<Move, int16_t>, int16_t> search(uint8_t depth, int16_t alpha=MIN_EVAL, int16_t beta=MAX_EVAL);//std::conditional_t evaluates at compile and returns the first type if the condition is true and the second type if the condition is false.
};
