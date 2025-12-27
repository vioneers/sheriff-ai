#pragma once

// Utilities for bitborads
// Predefined move dictionaries and magic numbers go here

// TODO: figure out a better place to put initialization functions

#include <array>
#include <bit>

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

        attacks |= (bb & notH) << 17;
        attacks |= (bb & notA) << 15;
        attacks |= (bb & notGH) << 10;
        attacks |= (bb & notAB) << 6;
        attacks |= (bb & notA) >> 17;
        attacks |= (bb & notH) >> 15;
        attacks |= (bb & notAB) >> 10;
        attacks |= (bb & notGH) >> 6;

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

constexpr int pop_lsb(uint64_t &b)
{
	auto index = std::countr_zero(b);
    b &= b - 1;
    return index;
}

// generate Bitboard of blocker pieces from index with `bits` nr of bits and movement mask m.
// used to find all blocker configurations by iterating over index
constexpr Bitboard index_to_blockers(int index, int bits, Bitboard m) 
{
	Bitboard result = 0ULL;
	for(int i = 0; i < bits; i++) {
		int j = pop_lsb(m);
		if(index & (1 << i)) result |= (1ULL << j);
	}
	return result;
}


// given a mask of blocker pieces, generate where rook / bishop can move (in an inneficient manner)
// result is up to and INCLUDING blocker pieces
// used to test if magic number is good and generate magic lookup tables
inline constexpr Bitboard ratt(int sq, Bitboard block) {
  Bitboard result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1; r <= 7; r++) {
    result |= (1ULL << (fl + r*8));
    if(block & (1ULL << (fl + r*8))) break;
  }
  for(r = rk-1; r >= 0; r--) {
    result |= (1ULL << (fl + r*8));
    if(block & (1ULL << (fl + r*8))) break;
  }
  for(f = fl+1; f <= 7; f++) {
    result |= (1ULL << (f + rk*8));
    if(block & (1ULL << (f + rk*8))) break;
  }
  for(f = fl-1; f >= 0; f--) {
    result |= (1ULL << (f + rk*8));
    if(block & (1ULL << (f + rk*8))) break;
  }
  return result;
}

inline constexpr Bitboard batt(int sq, Bitboard block) {
  Bitboard result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1, f = fl+1; r <= 7 && f <= 7; r++, f++) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk+1, f = fl-1; r <= 7 && f >= 0; r++, f--) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk-1, f = fl+1; r >= 0 && f <= 7; r--, f++) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk-1, f = fl-1; r >= 0 && f >= 0; r--, f--) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  return result;
}

// apply hashing of bitboard `b` with magic number.
// `bits` represents the nuber of usefull positions that need to be transposed from b to the hash (aka the shift).
inline constexpr int apply_magic(Bitboard b, uint64_t magic, int bits)
{
	return (int)((b*magic) >> (64 - bits));
}

// get positions we need to check for blockers when moving rook / bishop
inline constexpr std::array<Bitboard, 64> rmask()
{
	std::array<Bitboard, 64> mask = {};
	for(int sq = 0; sq < 64; sq++)
	{
		int rk = sq / 8;
		int fl = sq % 8;
		int r, f;

		for(r = rk+1; r <= 6; r++) mask[sq] |= (1ULL << (fl + r*8));
		for(r = rk-1; r >= 1; r--) mask[sq] |= (1ULL << (fl + r*8));
		for(f = fl+1; f <= 6; f++) mask[sq] |= (1ULL << (f + rk*8));
		for(f = fl-1; f >= 1; f--) mask[sq] |= (1ULL << (f + rk*8));
	}
	return mask;
}

inline constexpr std::array<Bitboard, 64> bmask()
{
	std::array<Bitboard, 64> mask = {};
	for(int sq = 0; sq < 64; sq++)
	{
		int rk = sq / 8;
		int fl = sq % 8;
		int r, f;

		for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) mask[sq] |= (1ULL << (f + r*8));
		for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) mask[sq] |= (1ULL << (f + r*8));
		for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) mask[sq] |= (1ULL << (f + r*8));
		for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) mask[sq] |= (1ULL << (f + r*8));
	}
	return mask;
}

inline constexpr std::array<Bitboard, 64> RookMask = rmask();
inline constexpr std::array<Bitboard, 64> BishopMask = bmask();

#ifndef MAKE_MAGIC
// generate magic lookup table for rook and bishop (indexed by square and magic hash)
inline constexpr std::array<std::array<Bitboard, 4096>, 64> magicLookUpGen(int bishop)
{
	std::array<std::array<Bitboard, 4096>, 64> magicLookUp;
	for(int sq = 0; sq < 64; sq ++)
	{
		Bitboard mask = bishop? BishopMask[sq] : RookMask[sq];
		int shift = bishop ? BShift[sq] : RShift[sq];

		// generate all blocker configurations and the positions we can move to for each
		Bitboard block[4096], attack[4096]; // we have at most 12 positions we are interested in, so 2^12 masks

		for (int i = 0; i < (1<<shift); i++)
		{
			block[i] = index_to_blockers(i, shift, mask);
			attack[i] = bishop? batt(sq, block[i]) : ratt(sq, block[i]);
			
			uint64_t magic = bishop? BMagic[sq] : RMagic[sq];

			uint64_t hash = apply_magic(block[i], magic, shift);
			magicLookUp[sq][hash] = attack[i];
		}
	}
	
	return magicLookUp;
}

const auto RookAttacks = magicLookUpGen(false);
const auto BishopAttacks = magicLookUpGen(true);
#endif
