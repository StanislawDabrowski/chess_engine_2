#include "Engine.h"
#include "MoveGenerator.h"
#include "Color.h"
#include "MoveOrdering.h"

Engine::Engine()
	:board(), mg(&board), se(&board, &mg)
{
	StaticEval::initialize_static_members();
	TT_size = DEFAULT_TT_SIZE;
	update_TT_size();
	normal_search_nodes_searched = 0;
	quiescence_search_nodes_searched = 0;
	TT_hits = 0;
	TT_writes = 0;
	stop_search.store(false, std::memory_order_relaxed);
}

Engine::Engine(const Engine& other)
	:board(other.board),
	mg(&board),
	se(&board, &mg),
	normal_search_nodes_searched(other.normal_search_nodes_searched),
	quiescence_search_nodes_searched(other.quiescence_search_nodes_searched),
	TT_hits(0),
	TT_writes(0),
	TT_size(other.TT_size),
	stop_search(false)
{
	update_TT_size();
}

Engine& Engine::operator=(const Engine& other)
{
	board = other.board;
	mg = MoveGenerator(&board);
	se = StaticEval(&board, &mg);
	normal_search_nodes_searched = other.normal_search_nodes_searched;
	quiescence_search_nodes_searched = other.quiescence_search_nodes_searched;
	TT_hits = 0;
	TT_writes = 0;
	TT_size = other.TT_size;
	update_TT_size();
	stop_search.store(false, std::memory_order_relaxed);
	return *this;
}

void Engine::update_TT_size()
{
	TT.resize(TT_size);
}

template<Color color>
uint64_t Engine::perft(uint8_t depth)
{
	if (depth >= 3)
	{
		if (stop_search.load(std::memory_order_relaxed))
			return 0;
	}
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
	if (stop_search.load(std::memory_order_relaxed))
	{
		if constexpr (root)
			return std::pair<Move, int16_t>(0, 0);
		else
			return MAX_EVAL;//we return max eval so the parent node will not choose that node, since it's good for us it's bad for them
	}
	if ((qsearch && quiescence_search_nodes_searched % 2048 == 0) || (!qsearch && normal_search_nodes_searched % 2048 == 0))
	{
		if (std::chrono::high_resolution_clock::now() >= search_time_hard_bound)
		{
			stop_search.store(true, std::memory_order_relaxed);
			if constexpr (root)
				return std::pair<Move, int16_t>(0, 0);
			else
				return MAX_EVAL;
		}
	}
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
	if (depth == 0 && !qsearch)
	{
		if constexpr (root)
			return std::pair<Move, int16_t>(0, search<color, false, true, count_searched_nodes>(0, alpha, beta));
		else
			return search<color, false, true, count_searched_nodes>(0, alpha, beta);
	}
	
	int16_t best_score = MIN_EVAL - 1;
	TTEntry *tt_entry = &TT[board.positions_stack[board.current_position_idx].hash%TT_size];
	if (tt_entry->hash == board.positions_stack[board.current_position_idx].hash && tt_entry->depth >= depth)
	{
		switch (tt_entry->eval_type)
		{
		case TTEvalType::Exact:
			if constexpr (root)
				return std::make_pair(tt_entry->best_move, tt_entry->eval);
			else
				return tt_entry->eval;
			break;
		case TTEvalType::LowerBound:
			best_score = tt_entry->eval;
			alpha = std::max(alpha, best_score);
			break;
		case TTEvalType::UpperBound:
			beta = std::min(beta, tt_entry->eval);
			break;
		}
		if (alpha >= beta)
		{
			if constexpr (root)
			{
				;//eval type is never exact if we are here and we don't want to return in root if we don't have the actaully best move
			}
			else
				return tt_entry->eval;
		}
	}
	
	//null move pruning
	if constexpr (!qsearch && !root)
	{
		if (!mg.in_check<color>())
		{
			board.make_null_move();
			int16_t score = -search<color==White ? Black : White, false, false, count_searched_nodes>(depth - 1, -beta, -beta + 1);
			board.unmake_move();
			if (score >= beta)
			{
				return score;
			}
		}
	}

	int16_t original_alpha = alpha;

	if constexpr (qsearch)
		mg.generate_noisy_pseudo_legal_moves<color>();
	else
		mg.generate_pseudo_legal_moves<color>();
	mg.filter_pseudo_legal_moves<color>();

	if (board.positions_stack[board.current_position_idx].legal_moves_length == 0)
	{
		//if not in qsearch it's checkmate or stalemate
		int16_t eval;
		if (mg.checks)//checks is set in filter_pseudo_legal_moves so it can be used here
			eval = MIN_EVAL;
		else
			if constexpr (qsearch)
				eval = se.evaluate<color>();
			else
				eval = 0;
		if constexpr (root)
			return std::pair<Move, int16_t>(0, eval);
		else
			return eval;
	}
	if constexpr (qsearch)
	{
		if (!mg.checks)
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
	}

	Move tt_move = 0;
	
	//move ordering
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_moves_length;++i)
	{
		if (tt_entry->best_move == board.positions_stack[board.current_position_idx].legal_moves[i])
		{
			++TT_hits;
			board.positions_stack[board.current_position_idx].move_ordering_scores[i] = 32767;
			continue;
		}
		if (mg.checks)
		{
			board.positions_stack[board.current_position_idx].move_ordering_scores[i] = MoveOrdering::MoveType_score_in_check[board.positions_stack[board.current_position_idx].legal_moves[i] >> 12];
			continue;
		}
		if constexpr (qsearch)
		{
			board.positions_stack[board.current_position_idx].move_ordering_scores[i] = MoveOrdering::MoveType_score_qsearch_no_check[board.positions_stack[board.current_position_idx].legal_moves[i] >> 12];
		}
		else
		{
			board.positions_stack[board.current_position_idx].move_ordering_scores[i] = MoveOrdering::MoveType_score_no_check[board.positions_stack[board.current_position_idx].legal_moves[i] >> 12];
		}
	}
	
	//sort moves with insertion sort
	for (int i = 1;i<board.positions_stack[board.current_position_idx].legal_moves_length;++i)
	{
		int j = i;
		while (j > 0 && board.positions_stack[board.current_position_idx].move_ordering_scores[j] > board.positions_stack[board.current_position_idx].move_ordering_scores[j-1])
		{
			std::swap(board.positions_stack[board.current_position_idx].move_ordering_scores[j], board.positions_stack[board.current_position_idx].move_ordering_scores[j-1]);
			std::swap(board.positions_stack[board.current_position_idx].legal_moves[j], board.positions_stack[board.current_position_idx].legal_moves[j-1]);
			--j;
		}
	}
	Move best_move = 0;	
	for (int i = 0;i<board.positions_stack[board.current_position_idx].legal_moves_length;++i)
	{
		board.make_move(board.positions_stack[board.current_position_idx].legal_moves[i]);
		int16_t score = -search<color==White ? Black : White, false, qsearch, count_searched_nodes>(depth - (qsearch ? 0 : 1), -beta, -alpha);
		
		if (score > best_score)
		{
			best_score = score;
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

	int16_t best_score_for_tt_conditions = best_score;
	if (std::abs(best_score) > MATE_THRESHOLD)
	{
		if (best_score > 0)
			best_score -= 1;//to prefer faster wins
		else if (best_score < 0)
			best_score += 1;//to prefer slower losses
	}
	if (best_move == 0)//no move better then the evaluation from the TT was found (eval type must have been lower bound)
		best_move = tt_entry->best_move;

	//store in TT
	if constexpr (!qsearch)
	{
		tt_entry->hash = board.positions_stack[board.current_position_idx].hash;
		tt_entry->best_move = best_move;
		tt_entry->depth = depth;
		tt_entry->eval = best_score;
		if (best_score_for_tt_conditions >= beta)
			tt_entry->eval_type = TTEvalType::LowerBound;
		else if (best_score_for_tt_conditions <= original_alpha)
			tt_entry->eval_type = TTEvalType::UpperBound;
		else
			tt_entry->eval_type = TTEvalType::Exact;
		++TT_writes;
	}
	if constexpr (root)
		return std::pair<Move, int16_t>(best_move, best_score);
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
