#pragma once
#include "Board.h"
#include "MoveGenerator.h"


class Engine
{
private:


public:
	Board board;
	MoveGenerator mg;

	Engine();
	uint64_t perft(uint8_t depth);
};
