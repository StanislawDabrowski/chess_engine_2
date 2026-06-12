#pragma once
#include "Defs.h"


struct TTEntry
{
	uint64_t hash;
	Move best_move;
	TTEntry() :hash(0), best_move(0) { }
};
