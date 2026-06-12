#pragma once
#include "Defs.h"


struct TTEntry
{
	Move best_move;
	TTEntry() :best_move(0) { }
};
