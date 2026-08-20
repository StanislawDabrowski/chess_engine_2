#pragma once
#include "Board.h"
#include "MoveGenerator.h"
#include "StaticEval.h"
#include "TTEntry.h"
#include <atomic>
#include <chrono>


class Engine
{
public:
	Board board;
	MoveGenerator mg;
	StaticEval se;
	uint64_t TT_size;//number of entries in the TT
	std::vector<TTEntry> TT;
	std::atomic<bool> stop_search;
	uint64_t normal_search_nodes_searched;
	uint64_t quiescence_search_nodes_searched;
	std::chrono::time_point<std::chrono::high_resolution_clock> search_time_hard_bound;
	uint64_t TT_hits;
	uint64_t TT_writes;

	static constexpr int16_t MAX_EVAL = 32767;
	static constexpr int16_t MIN_EVAL = -32767;//needs to be -32767 so -MIN_EVAL is MAX_EVAL, not itself, which due to integer overflow would probably (it's UB), be the case
	static constexpr uint8_t MAX_DEPTH = 255;//max depth for normal search (excludes qsearch, qsearch is unbounded)
	static constexpr int16_t MATE_THRESHOLD = 30000;//assumes maximum depth of qsearch to be at maximum 2512

	static constexpr uint64_t DEFAULT_TT_SIZE = (16*1024*1024)/sizeof(TTEntry);//16MB

	Engine();
	Engine(const Engine&);
	Engine& operator=(const Engine&);
	void update_TT_size();//updates the size of the TT vector to current TT_size
	template<Color color>
	uint64_t perft(uint8_t depth);
	uint8_t calculate_reduction_for_lmr(uint8_t move_index,uint8_t depth);
	template<Color color, bool root = false, bool qsearch=false, bool count_searched_nodes = false>//if root, return a pair of the best move and evaluation, otherwise return only the evaluation. Made that way to overheadlessly have only 1 definition of the search function for both root and non-root calls.
	requires(!(qsearch && root))
	std::conditional_t<root, std::pair<Move, int16_t>, int16_t> search(uint8_t depth, int16_t alpha=MIN_EVAL, int16_t beta=MAX_EVAL);//std::conditional_t evaluates at compile and returns the first type if the condition is true and the second type if the condition is false.
};
