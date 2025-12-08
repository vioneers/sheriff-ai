#include "types.h"

// Utilities for bitborads
// Predefined move dictionaries and magic numbers go here

// General shift function
template <int Shift>
uint64_t shift(uint64_t bb) {
    if (Shift > 0) return bb << Shift;
    else return bb >> (-Shift);
}

// We use constexpr to evaluate at compile time and not runtime
constexpr Bitboard[8] RankMasks = {
	Bitboard[8] arr;
	Bitboard rank1 = (1ULL << 8) - 1;
	
	for(int i=0; i<8; ++i)
		arr[i] = rank1 << (8 * i);

	return arr;
}

constexpr Bitboard[8] FileMasks = {
	Bitboard[8] arr;
	
	for(int i=0; i<8; ++i)
		for(int j=0; j<8; ++j)
			arr[i] |= 1ULL << (8*j + i);

	return arr;
}
