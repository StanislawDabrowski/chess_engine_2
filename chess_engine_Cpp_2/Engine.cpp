#include "Engine.h"
#include "Color.h"


Engine::Engine()
	:board(), mg(&board), se(&board)
{
	mg.initialize_attack_tables();
}

template<Color color>
uint64_t Engine::perft(uint8_t depth)
{
	mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	
	if (depth == 1) return board.positions_stack[board.current_position_idx].legal_move_next_idx;
	uint64_t count = 0;
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_move_next_idx;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		count+=perft<color==White ? Black : White>(depth-1);
		board.unmake_move();
	}
	return count;

}

template<Color color>
std::pair<Move, uint16_t> Engine::search(uint8_t depth)
{
	//return a move after which static evaluation is the best, without any search, just 1 ply.
	mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	int16_t best_score = (color == White) ? -32768 : 32767;
	Move best_move = 0;
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_move_next_idx;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		int16_t score = se.evaluate();
		if ((color == White && score > best_score) || (color == Black && score < best_score))
		{
			best_score = score;
			best_move = board.positions_stack[board.current_position_idx].legal_moves[i];
		}
		board.unmake_move();
	}
	return {best_move, best_score};
}



template uint64_t Engine::perft<White>(uint8_t depth);
template uint64_t Engine::perft<Black>(uint8_t depth);

template std::pair<Move, uint16_t> Engine::search<White>(uint8_t depth);
template std::pair<Move, uint16_t> Engine::search<Black>(uint8_t depth);
