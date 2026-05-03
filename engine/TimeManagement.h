#pragma once
#include <cstdint>
#include "Engine.h"

uint64_t get_time_to_think_in_ms(Engine* engine, uint64_t wtime, uint64_t btime, uint64_t winc, uint64_t binc, int32_t movestogo=-1);
