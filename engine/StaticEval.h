#pragma once
#include "Board.h"
#include "Color.h"



class StaticEval
{
public:
	Board* board;
	static constexpr uint16_t piece_values[6] = { 100, 300, 300, 500, 900, 0 };//pawn knight bishop rook queen king
	StaticEval(Board* board);
	template<Color color>
	int16_t evaluate();
};
