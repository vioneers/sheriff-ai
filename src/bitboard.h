#pragma once

// Utilities for bitborads
// Predefined move dictionaries and magic numbers go here

#include <array>

#include "types.h"

#ifndef MAKE_MAGIC
#include "magic.h"
#endif

// General shift function
template <int Shift>
uint64_t shift(uint64_t bb) {
    if (Shift > 0) return bb << Shift;
    else return bb >> (-Shift);
}

// We use consteval to force evaluate functions at compile time and not runtime
consteval std::array<Bitboard, 8> genRankMask()
{
	std::array<Bitboard, 8> arr = {};
	Bitboard rank1 = (1ULL << 8) - 1;
	
	for(int i=0; i<8; ++i)
		arr[i] = rank1 << (8 * i);
	
	return arr;
}

consteval std::array<Bitboard, 8> genFileMask()
{
	std::array<Bitboard, 8> arr = {};
	
	for(int i=0; i<8; ++i)
		for(int j=0; j<8; ++j)
			arr[i] |= 1ULL << (8*j + i);

	return arr;
}

constexpr std::array<Bitboard, 8> RankMask = genRankMask();
constexpr std::array<Bitboard, 8> FileMask = genFileMask();
