#pragma once
#include "Board.h"
#include "MoveGenerator.h"
#include "Color.h"



class StaticEval
{
public:
	Board* board;
	MoveGenerator* mg;
	static constexpr uint16_t piece_values[6] = { 100, 300, 300, 500, 900, 0 };//pawn knight bishop rook queen king
	static constexpr int16_t mobility_score = 10;//score for each pseudo legal move
	StaticEval(Board* board, MoveGenerator* mg);
	template<Color color>
	int16_t evaluate_mobility();
	template<Color color>
	int16_t evaluate();
};
