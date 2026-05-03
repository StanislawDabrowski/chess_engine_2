#include "TimeManagement.h"


uint64_t get_time_to_think_in_ms(Engine* engine, uint64_t wtime, uint64_t btime, uint64_t winc, uint64_t binc, int32_t movestogo) {
	if (movestogo == -1) {
		movestogo = 40;
	}

	uint64_t time_left = engine->board.side_to_move==White ? wtime : btime;
	uint64_t inc = engine->board.side_to_move==White ? winc : binc;

	uint64_t time_to_think = time_left / movestogo + inc;

	if (time_to_think > time_left*0.4)
		time_to_think = time_left*0.4;//very large margin to avoid flagging with current very weak time management

	return time_to_think;
}
