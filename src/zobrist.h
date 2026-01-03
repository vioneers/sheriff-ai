#pragma once
#include <cstdint>

extern uint64_t Z_PSQ[12][64];
extern uint64_t Z_TURN; 
extern uint64_t Z_CASTLE[16];
extern uint64_t Z_EPFILE[9];

void init_zobrist(uint64_t seed = 0xC0FFEE123456789ULL);