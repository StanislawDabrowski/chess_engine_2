#include "StaticEval.h"
#include "Color.h"
#include "PieceType.h"
#include "Utils.h"



bool StaticEval::static_members_initialized = false;
Bitboard StaticEval::squares_1_in_front[2][64];
Bitboard StaticEval::squares_2_in_front[2][64];

StaticEval::StaticEval(Board* board, MoveGenerator* mg)
	:board(board), mg(mg)
{ }

void StaticEval::initialize_static_members()
{
	if (static_members_initialized)
	{
		return;
	}
	static_members_initialized = true;
	Bitboard temp;
	for (size_t i = 0; i < 64; ++i)
	{
		temp = 0;
		if (i >= 56)
		{
			squares_1_in_front[White][i] = temp;
			continue;
		}
		if (i % 8 != 0)
		{
			temp |= 1ULL << (i + 7);
		}
		if (i % 8 != 7)
		{
			temp |= 1ULL << (i + 9);
		}
		temp |= 1ULL << (i + 8);
		squares_1_in_front[White][i] = temp;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		temp = 0;
		if (i < 8)
		{
			squares_1_in_front[Black][i] = temp;
			continue;
		}
		if (i % 8 != 0)
		{
			temp |= 1ULL << (i - 9);
		}
		if (i % 8 != 7)
		{
			temp |= 1ULL << (i - 7);
		}
		temp |= 1ULL << (i - 8);
		squares_1_in_front[Black][i] = temp;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		temp = 0;
		if (i >= 48)
		{
			squares_2_in_front[White][i] = temp;
			continue;
		}
		if (i % 8 != 0)
		{
			temp |= 1ULL << (i + 15);
		}
		if (i % 8 != 7)
		{
			temp |= 1ULL << (i + 17);
		}
		temp |= 1ULL << (i + 16);
		squares_2_in_front[White][i] = temp;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		temp = 0;
		if (i < 16)
		{
			squares_2_in_front[Black][i] = temp;
			continue;
		}
		if (i % 8 != 0)
		{
			temp |= 1ULL << (i - 17);
		}
		if (i % 8 != 7)
		{
			temp |= 1ULL << (i - 15);
		}
		temp |= 1ULL << (i - 16);
		squares_2_in_front[Black][i] = temp;
	}
}


template<Color color>
int16_t StaticEval::evaluate_mobility()
{
	//returns score for mobility
	//mobility is calculated as number of pseudo legal moves but also couning attacks on pieces of the same color and for pawns also squares they can attack even when there are no pieces, because of that captures for pawns are divided by 2
	Color opp = color == White ? Black : White;
	int16_t score = 0;

	Bitboard piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Pawn];

	//pawns
	Bitboard single_push = (color == White ? ((piece_copy & Utils::RANK_7_NEGATION) << 8) : ((piece_copy & Utils::RANK_2_NEGATION) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;
	Bitboard double_push = (color == White ? ((single_push & Utils::RANK_3) << 8) : ((single_push & Utils::RANK_6) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;

	//left/right captures are a captures to the specific side always relative to white
	Bitboard left_captures = (color == White ? ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_7_NEGATION) << 7) : ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_2_NEGATION) >> 9));
	Bitboard right_captures = (color == White ? ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_7_NEGATION) << 9) : ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_2_NEGATION) >> 7));

	Bitboard push_promotion = (color == White ? ((piece_copy & Utils::RANK_7) << 8) : ((piece_copy & Utils::RANK_2) >> 8)) & ~board->positions_stack[board->current_position_idx].all_pieces;
	Bitboard left_capture_promotion = (color == White ? ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_7) << 7) : ((piece_copy & Utils::FILE_A_NEGATION & Utils::RANK_2) >> 9));
	Bitboard right_capture_promotion = (color == White ? ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_7) << 9) : ((piece_copy & Utils::FILE_H_NEGATION & Utils::RANK_2) >> 7));

	score += (std::popcount(single_push) + std::popcount(double_push) + std::popcount(push_promotion)) * mobility_score;
	score += ((std::popcount(left_captures) + std::popcount(right_captures) + std::popcount(left_capture_promotion) + std::popcount(right_capture_promotion)) * mobility_score) >> 1;//capture posibilities are worth less beacuse moving there without a capture is not possible and most of these are not actually possible at the moment, there is not piece to capture. this is only a special case of pawns

	//en passant
	uint8_t en_passant_square = board->positions_stack[board->current_position_idx].en_passant_square;
	Bitboard en_passant_mask;//can be left uninitialized since pawns_left and pawns_right are 0 so AND will yell 0 anyway, and these can be different form 0 only if en_passant_mask get set bofer as well
	Bitboard pawns_left = 0;
	Bitboard pawns_right = 0;
	if (color == Black && en_passant_square >= 16 && en_passant_square < 24)
	{
		en_passant_mask = 1ULL << en_passant_square;
		pawns_left = (piece_copy & Utils::FILE_H_NEGATION) >> 7;
		pawns_right = (piece_copy & Utils::FILE_A_NEGATION) >> 9;
	}
	else if (color == White && en_passant_square >= 40 && en_passant_square < 48)
	{
		en_passant_mask = 1ULL << en_passant_square;
		pawns_left = (piece_copy & Utils::FILE_H_NEGATION) << 9;
		pawns_right = (piece_copy & Utils::FILE_A_NEGATION) << 7;
	}
	if (pawns_left & en_passant_mask)
	{
		score += mobility_score;
	}
	if (pawns_right & en_passant_mask)
	{
		score += mobility_score;
	}

	size_t from;
	Bitboard attacks;
	//knights
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Knight];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		attacks = mg->knight_attack_tables[from];
		score += std::popcount(attacks) * mobility_score;
		piece_copy &= piece_copy - 1;
	}
	//bishops
	Bitboard relevant_blockers;
	size_t index;
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Bishop];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & mg->bishop_relevant_blockers[from];
		index = (relevant_blockers * mg->bishop_magic_numbers[from]) >> (64 - std::popcount(mg->bishop_relevant_blockers[from]));
		attacks = mg->bishop_attack_tables[from][index];
		score += std::popcount(attacks) * mobility_score;
		piece_copy &= piece_copy - 1;
	}
	//rooks
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Rook];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & mg->rook_relevant_blockers[from];
		index = (relevant_blockers * mg->rook_magic_numbers[from]) >> (64 - std::popcount(mg->rook_relevant_blockers[from]));
		attacks = mg->rook_attack_tables[from][index];
		score += std::popcount(attacks) * mobility_score;
		piece_copy &= piece_copy - 1;
	}
	//queens
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][Queen];
	while (piece_copy)
	{
		from = std::countr_zero(piece_copy);
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & mg->bishop_relevant_blockers[from];
		index = (relevant_blockers * mg->bishop_magic_numbers[from]) >> (64 - std::popcount(mg->bishop_relevant_blockers[from]));
		attacks = mg->bishop_attack_tables[from][index];
		relevant_blockers = board->positions_stack[board->current_position_idx].all_pieces & mg->rook_relevant_blockers[from];
		index = (relevant_blockers * mg->rook_magic_numbers[from]) >> (64 - std::popcount(mg->rook_relevant_blockers[from]));
		attacks |= mg->rook_attack_tables[from][index];
		score += std::popcount(attacks) * mobility_score;
		piece_copy &= piece_copy - 1;
	}
	//king
	piece_copy = board->positions_stack[board->current_position_idx].pieces[color][King];
	from = std::countr_zero(piece_copy);
	attacks = mg->king_attack_tables[from];
	score += std::popcount(attacks) * mobility_score;

	return score;
}

template<Color color>
int16_t StaticEval::evaluate_king_safety()
{
	int16_t score = 0;
	uint8_t king_square = std::countr_zero(board->positions_stack[board->current_position_idx].pieces[color][King]);
	Bitboard squares_of_intreset = squares_1_in_front[color][king_square];
	score += std::popcount(squares_of_intreset & board->positions_stack[board->current_position_idx].pieces[color][Pawn]) * score_for_panws_1_in_front_of_king;
	squares_of_intreset = squares_2_in_front[color][king_square];
	score += std::popcount(squares_of_intreset & board->positions_stack[board->current_position_idx].pieces[color][Pawn]) * score_for_pawns_2_in_front_of_king;
	return score;
}

template<Color color>
int16_t StaticEval::evaluate()
{
	constexpr Color opp = color == White ? Black : White;
	int16_t score = 0;
	for (int color_iter = 0; color_iter < 2; ++color_iter)
	{
		for (int piece_type = 0; piece_type < 6; ++piece_type)
		{
			score += piece_values[piece_type] * std::popcount(board->positions_stack[board->current_position_idx].pieces[color_iter][piece_type]) * (color_iter == color ? 1 : -1);
		}
	}

	score += evaluate_mobility<color>();
	score -= evaluate_mobility<opp>();
	score += evaluate_king_safety<color>();
	score -= evaluate_king_safety<opp>();
	
	return score;
}



template int16_t StaticEval::evaluate<White>();
template int16_t StaticEval::evaluate<Black>();
