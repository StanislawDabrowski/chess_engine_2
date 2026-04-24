#include "StaticEval.h"
#include "Color.h"


StaticEval::StaticEval(Board* board)
	:board(board)
{ }


int16_t StaticEval::evaluate()
{
	int16_t score = 0;
	for (int color = 0; color < 2; ++color)
	{
		for (int piece_type = 0; piece_type < 6; ++piece_type)
		{
			score += piece_values[piece_type] * std::popcount(board->positions_stack[board->current_position_idx].pieces[color][piece_type]) * (color == White ? 1 : -1);
		}
	}
	return score;
}
