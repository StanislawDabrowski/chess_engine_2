#include "Engine.h"
#include "Color.h"


Engine::Engine()
	:board(), mg(&board)
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
	//generate random move
	mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	return {board.positions_stack[board.current_position_idx].legal_moves[rand()%board.positions_stack[board.current_position_idx].legal_move_next_idx], 0};
}



template uint64_t Engine::perft<White>(uint8_t depth);
template uint64_t Engine::perft<Black>(uint8_t depth);

template std::pair<Move, uint16_t> Engine::search<White>(uint8_t depth);
template std::pair<Move, uint16_t> Engine::search<Black>(uint8_t depth);
