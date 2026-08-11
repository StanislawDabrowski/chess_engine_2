#pragma once
#include "Defs.h"
#include "TTEvalType.h"


struct TTEntry
{
	int64_t hash;
	Move best_move;
	int16_t eval;
	uint8_t depth;
	TTEvalType eval_type;
	TTEntry();
};
