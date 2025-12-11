#pragma once

#include "types.h"

// Utilities for bitborads
// Predefined move dictionaries and magic numbers go here

// General shift function
template <int Shift>
uint64_t shift(uint64_t bb) {
    if (Shift > 0) return bb << Shift;
    else return bb >> (-Shift);
}

const Bitboard RANK_1 = 0x00000000000000FFULL;
const Bitboard RANK_2 = 0x000000000000FF00ULL;
const Bitboard RANK_3 = 0x0000000000FF0000ULL;
const Bitboard RANK_4 = 0x00000000FF000000ULL;
const Bitboard RANK_5 = 0x000000FF00000000ULL;
const Bitboard RANK_6 = 0x0000FF0000000000ULL;
const Bitboard RANK_7 = 0x00FF000000000000ULL;
const Bitboard RANK_8 = 0xFF00000000000000ULL;

const Bitboard FILE_A = 0x0101010101010101ULL;
const Bitboard FILE_B = 0x0202020202020202ULL;
const Bitboard FILE_C = 0x0404040404040404ULL;
const Bitboard FILE_D = 0x0808080808080808ULL;
const Bitboard FILE_E = 0x1010101010101010ULL;
const Bitboard FILE_F = 0x2020202020202020ULL;
const Bitboard FILE_G = 0x4040404040404040ULL;
const Bitboard FILE_H = 0x8080808080808080ULL;


