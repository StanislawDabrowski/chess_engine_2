#include "TTEntry.h"
#include "Engine.h"


TTEntry::TTEntry()
	: eval(Engine::MIN_EVAL), eval_type(TTEvalType::LowerBound), hash(0), depth(0), best_move(0)
{ }
