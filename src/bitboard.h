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
constexpr uint64_t shift(uint64_t bb) {
    return Shift > 0 ? bb << Shift : bb >> (-Shift);
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

inline constexpr std::array<Bitboard, 64> KnightAttacks = []() {
    constexpr Bitboard notA  = 0xFEFEFEFEFEFEFEFEULL;
    constexpr Bitboard notAB = 0xFCFCFCFCFCFCFCFCULL;
    constexpr Bitboard notH  = 0x7F7F7F7F7F7F7F7FULL;
    constexpr Bitboard notGH = 0x3F3F3F3F3F3F3F3FULL;

    std::array<Bitboard, 64> table{};
    for (int sq = 0; sq < 64; ++sq) {
        Bitboard bb = 1ULL << sq;
        Bitboard attacks = 0;

        attacks |= (bb << 17) & notH;
        attacks |= (bb << 15) & notA;
        attacks |= (bb << 10) & notGH;
        attacks |= (bb << 6)  & notAB;
        attacks |= (bb >> 17) & notA;
        attacks |= (bb >> 15) & notH;
        attacks |= (bb >> 10) & notAB;
        attacks |= (bb >> 6)  & notGH;

        table[sq] = attacks;
    }
    return table;
}();

inline constexpr std::array<Bitboard, 64> KingAttacks = []() {
    constexpr Bitboard notA = 0xFEFEFEFEFEFEFEFEULL;
    constexpr Bitboard notH = 0x7F7F7F7F7F7F7F7FULL;

    std::array<Bitboard, 64> table{};
    for (int sq = 0; sq < 64; ++sq) {
        Bitboard bb = 1ULL << sq;
        Bitboard attacks = 0;

        attacks |= (bb << 8);               // north
        attacks |= (bb >> 8);               // south
        attacks |= (bb << 1) & notA;        // east
        attacks |= (bb >> 1) & notH;        // west
        attacks |= (bb << 9) & notA;        // north-east
        attacks |= (bb << 7) & notH;        // north-west
        attacks |= (bb >> 9) & notH;        // south-west
        attacks |= (bb >> 7) & notA;        // south-east

        table[sq] = attacks;
    }
    return table;
}();
