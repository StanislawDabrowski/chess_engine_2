#pragma once
#include "Board.h"



class StaticEval
{
public:
	Board* board;
	static constexpr uint16_t piece_values[6] = { 100, 300, 300, 500, 900, 0 };//pawn knight bishop rook queen king
	StaticEval(Board* board);
	int16_t evaluate();
};
