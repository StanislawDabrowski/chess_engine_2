#pragma once
#include "Board.h"
#include "MoveGenerator.h"
#include "Color.h"



class StaticEval
{
public:
	Board* board;
	MoveGenerator* mg;
	static bool static_members_initialized;
	static constexpr uint16_t piece_values[6] = { 100, 300, 300, 500, 900, 0 };//pawn knight bishop rook queen king
	static constexpr int16_t mobility_score = 10;//score for each pseudo legal move
	static constexpr int16_t score_for_panws_1_in_front_of_king = 8;
	static constexpr int16_t score_for_pawns_2_in_front_of_king = 4;
	static Bitboard squares_1_in_front[2][64];//3 squares 1 rank above the king
	static Bitboard squares_2_in_front[2][64];//3 squares 2 ranks above the king
	StaticEval(Board* board, MoveGenerator* mg);
	static void initialize_static_members();
	template<Color color>
	int16_t evaluate_mobility();
	template<Color color>
	int16_t evaluate_king_safety();
	template<Color color>
	int16_t evaluate();
};
