#include "Engine.h"
#include "MoveGenerator.h"
#include "Color.h"

Engine::Engine()
	:board(), mg(&board), se(&board)
{
	mg.initialize_attack_tables();
	nodes_searched = 0;
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

template<Color color, bool root, bool count_searched_nodes>
std::conditional_t<root, std::pair<Move, int16_t>, int16_t> Engine::search(uint8_t depth, int16_t alpha, int16_t beta)
{
	if constexpr (count_searched_nodes)
		++nodes_searched;
	//negmax with alpha-beta pruning
	mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	if (board.positions_stack[board.current_position_idx].legal_move_next_idx == 0)
	{
		//checkmate or stalemate
		int16_t eval;
		if (mg.in_check<color>())
			eval = MIN_EVAL;
		else
			eval = 0;
		if constexpr (root)
			return std::pair<Move, int16_t>(0, color == White ? eval : -eval);
		else
			return eval;
	}
	if (depth == 0)
	{
		if constexpr (root)
			return std::pair<Move, int16_t>(0, se.evaluate<color>());
		else
			return se.evaluate<color>();
	}
	int16_t best_score = MIN_EVAL;
	Move best_move;	
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_move_next_idx;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		int16_t score = -search<color==White ? Black : White, false, count_searched_nodes>(depth - 1, -beta, -alpha);
		if (score > best_score)
		{
			best_score = score;
			if constexpr (root)
				best_move = board.positions_stack[board.current_position_idx-1].legal_moves[i];
			if (best_score > alpha)
			{
				alpha = best_score;
				if (alpha >= beta)
				{
					board.unmake_move();
					break;//beta cutoff
				}
			}
		}
		board.unmake_move();
	}
	if constexpr (root)
		return std::pair<Move, int16_t>(best_move, color == White ? best_score : -best_score);
	else
		return best_score;
}




template uint64_t Engine::perft<White>(uint8_t depth);
template uint64_t Engine::perft<Black>(uint8_t depth);

template std::pair<Move, int16_t> Engine::search<White, true>(uint8_t depth, int16_t alpha, int16_t beta);
template std::pair<Move, int16_t> Engine::search<Black, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<White, false>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<Black, false>(uint8_t depth, int16_t alpha, int16_t beta);
template std::pair<Move, int16_t> Engine::search<White, true, true>(uint8_t depth, int16_t alpha, int16_t beta);
template std::pair<Move, int16_t> Engine::search<Black, true, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<White, false, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<Black, false, true>(uint8_t depth, int16_t alpha, int16_t beta);
