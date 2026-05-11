#include "Engine.h"
#include "MoveGenerator.h"
#include "Color.h"

Engine::Engine()
	:board(), mg(&board), se(&board, &mg)
{
	normal_search_nodes_searched = 0;
	quiescence_search_nodes_searched = 0;
}

Engine::Engine(const Engine& other)
	:board(other.board), mg(&board), se(&board, &mg), normal_search_nodes_searched(other.normal_search_nodes_searched), quiescence_search_nodes_searched(other.quiescence_search_nodes_searched)
{ }

Engine& Engine::operator=(const Engine& other)
{
	board = other.board;
	mg = MoveGenerator(&board);
	se = StaticEval(&board, &mg);
	normal_search_nodes_searched = other.normal_search_nodes_searched;
	quiescence_search_nodes_searched = other.quiescence_search_nodes_searched;
	return *this;
}

template<Color color>
uint64_t Engine::perft(uint8_t depth)
{
	mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	
	if (depth == 1) return board.positions_stack[board.current_position_idx].legal_moves_length;
	uint64_t count = 0;
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_moves_length;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		count+=perft<color==White ? Black : White>(depth-1);
		board.unmake_move();
	}
	return count;

}

template<Color color, bool root, bool qsearch, bool count_searched_nodes>
requires(!(qsearch && root))
std::conditional_t<root, std::pair<Move, int16_t>, int16_t> Engine::search(uint8_t depth, int16_t alpha, int16_t beta)
{
	if constexpr (count_searched_nodes)
	{
		if constexpr (qsearch)
			++quiescence_search_nodes_searched;
		else
			++normal_search_nodes_searched;
	}
	//negmax with alpha-beta pruning
	if (board.positions_stack[board.current_position_idx].draw_by_repetition || board.positions_stack[board.current_position_idx].halfmove_clock >= 100)
	{
		if constexpr (root)
			return std::pair<Move, int16_t>(0, 0);
		else
			return 0;
	}
	int16_t best_score;
	if constexpr (qsearch)
	{
		best_score = se.evaluate<color>();
		if (best_score >= beta)
		{
			return best_score;
		}
		if (best_score > alpha)
		{
			alpha = best_score;
		}
	}
	if constexpr (qsearch)
		mg.generate_noisy_pseudo_legal_moves<color>();
	else
		mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();
	if (board.positions_stack[board.current_position_idx].legal_moves_length == 0)
	{
		if constexpr (!qsearch)
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
	}
	if (depth == 0 && !qsearch)
	{
		if constexpr (root)
			return std::pair<Move, int16_t>(0, search<color, false, true, count_searched_nodes>(0, -beta, -alpha));
		else
			return search<color, false, true, count_searched_nodes>(0, -beta, -alpha);
	}

	if constexpr (!qsearch)
	{
		best_score = MIN_EVAL - 1;//MIN_EVAL is -2^15+1, so MIN_EVAL-1 does not wrap around. Is set to MIN_EVAL-1 for best_moves to be always initialized
	}
	
	Move best_move;	
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_moves_length;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		int16_t score = -search<color==White ? Black : White, false, qsearch, count_searched_nodes>(depth - (qsearch ? 0 : 1), -beta, -alpha);
		
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
	if (best_score > 0)
		best_score -= 1;//to prefer faster wins
	else if (best_score < 0)
		best_score += 1;//to prefer slower losses
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
template std::pair<Move, int16_t> Engine::search<White, true, false, true>(uint8_t depth, int16_t alpha, int16_t beta);
template std::pair<Move, int16_t> Engine::search<Black, true, false, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<White, false, false, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<Black, false, false, true>(uint8_t depth, int16_t alpha, int16_t beta);

template int16_t Engine::search<White, false, true, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<Black, false, true, true>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<White, false, true, false>(uint8_t depth, int16_t alpha, int16_t beta);
template int16_t Engine::search<Black, false, true, false>(uint8_t depth, int16_t alpha, int16_t beta);
