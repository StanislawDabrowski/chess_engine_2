#pragma once
#include "Board.h"
#include "MoveGenerator.h"
#include "StaticEval.h"


class Engine
{
private:


public:
	Board board;
	MoveGenerator mg;
	StaticEval se;

	Engine();
	template<Color color>
	uint64_t perft(uint8_t depth);
	template<Color color>
	std::pair<Move, uint16_t> search(uint8_t depth);
};
