#include "TimeManagement.h"


std::pair<uint64_t, uint64_t> get_time_to_think_in_ms(Engine* engine, uint64_t wtime, uint64_t btime, uint64_t winc, uint64_t binc, int32_t movestogo) {
	if (movestogo == -1) {
		movestogo = 40;
	}

	uint64_t time_left = engine->board.side_to_move==White ? wtime : btime;
	uint64_t inc = engine->board.side_to_move==White ? winc : binc;

	uint64_t lower_bound = time_left / movestogo + inc;
	uint64_t upper_bound = lower_bound * 2;
	if (lower_bound > time_left)
		lower_bound = time_left * 0.95;
	if (upper_bound > time_left)
		upper_bound = lower_bound;

	return std::make_pair(lower_bound, upper_bound);
}
