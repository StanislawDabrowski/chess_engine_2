#include "StaticEval.h"
#include "Color.h"


StaticEval::StaticEval(Board* board)
	:board(board)
{ }


template<Color color>
int16_t StaticEval::evaluate()
{
	int16_t score = 0;
	for (int color_iter = 0; color_iter < 2; ++color_iter)
	{
		for (int piece_type = 0; piece_type < 6; ++piece_type)
		{
			score += piece_values[piece_type] * std::popcount(board->positions_stack[board->current_position_idx].pieces[color_iter][piece_type]) * (color_iter == color ? 1 : -1);
		}
	}
	return score;
}



template int16_t StaticEval::evaluate<White>();
template int16_t StaticEval::evaluate<Black>();
