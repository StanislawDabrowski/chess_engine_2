#include "Engine.h"
#include "Color.h"
#include "MoveType.h"


Engine::Engine()
	:board(), mg(&board)
{
	mg.initialize_attack_tables();
}

uint64_t Engine::perft(uint8_t depth)
{
	if (board.side_to_move == White) 
	{
		mg.generate_pseudo_legal_moves<White>();
		mg.filter_pseudo_legal_moves<White>();
	}
	else
	{
		mg.generate_pseudo_legal_moves<Black>();
		mg.filter_pseudo_legal_moves<Black>();
	}
	if (depth == 1) return board.positions_stack[board.current_position_idx].legal_move_next_idx;
	uint64_t count = 0;
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_move_next_idx;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		count+=perft(depth-1);
		board.unmake_move();
	}
	return count;

}
