// This code generates the magic number used for rook and bishop movement
// It creates the magic.h file which needs to be included in piece.cpp

#include <iostream>
#include <fstream>
#include <random>
#include <limits>
#include <bit>

#define MAKE_MAGIC
#include "bitboard.h"

// let compiler optimize some stuff
#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("bmi,bmi2,lzcnt,popcnt")

// get random 64 bits as std::rand() only gives 15 random bits
uint64_t random_uint64() {
    // 1. Initialize the random device to get a seed (non-deterministic if supported)
	 std::random_device rd;
    
    // 2. Initialize the generator with the seed
    // mt19937_64 is a standard high-quality 64-bit generator
    static std::mt19937_64 gen(rd()); 

    // 3. Define the distribution (covers the full uint64 range)
    static std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    return dist(gen);
}

// prefer generating masks wuth few 1 bits for efficiency
uint64_t random_uint64_fewbits() {
  return random_uint64() & random_uint64() & random_uint64();
}

int pop_lsb(uint64_t &b)
{
	auto index = std::countr_zero(b);
    b &= b - 1;
    return index;
}

// generate Bitboard of blocker pieces from index with `bits` nr of bits and movement mask m.
// used to find all blocker configurations by iterating over index
Bitboard index_to_blockers(int index, int bits, Bitboard m) 
{
	Bitboard result = 0ULL;
	for(int i = 0; i < bits; i++) {
		int j = pop_lsb(m);
		if(index & (1 << i)) result |= (1ULL << j);
	}
	return result;
}

// get positions we need to check for blockers when moving rook / bishop
Bitboard rmask(int sq)
{
	Bitboard mask = 0ULL;
	int rk = sq / 8;
	int fl = sq % 8;
	int r, f;

	for(r = rk+1; r <= 6; r++) mask |= (1ULL << (fl + r*8));
	for(r = rk-1; r >= 1; r--) mask |= (1ULL << (fl + r*8));
	for(f = fl+1; f <= 6; f++) mask |= (1ULL << (f + rk*8));
	for(f = fl-1; f >= 1; f--) mask |= (1ULL << (f + rk*8));
	
	return mask;
}

Bitboard bmask(int sq)
{
	Bitboard mask = 0ULL;
	int rk = sq / 8;
	int fl = sq % 8;
	int r, f;

	for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) mask |= (1ULL << (f + r*8));
	for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) mask |= (1ULL << (f + r*8));
	for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) mask |= (1ULL << (f + r*8));
	for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) mask |= (1ULL << (f + r*8));

	return mask;
}

// given a mask of blocker pieces, generate where rook / bishop can move
// used to test if magic number is good
Bitboard ratt(int sq, Bitboard block) {
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

Bitboard batt(int sq, Bitboard block) {
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
// `bits` represents the nuber of usefull positions that need to be transposed from b to the hash.
int apply_magic(Bitboard b, uint64_t magic, int bits)
{
	return (int)((b*magic) >> (64 - bits));
}

// find magic number for given square by trial and error
uint64_t find_magic(int sq, bool bishop)
{
	Bitboard mask = bishop? bmask(sq) : rmask(sq);
	int n = std::popcount(mask); // get number of 1 bits in mask
	
	// generate all blocker configurations and the positions we can move to for each
	Bitboard block[4096], move[4096], used[4096]; // we have at most 12 positions we are interested in, so 2^12 masks

	for (int i = 0; i < (1<<n); i++)
	{
		block[i] = index_to_blockers(i, n, mask);
		move[i] = bishop? batt(sq, block[i]) : ratt(sq, block[i]);
	}

	for(int k = 0; k < 100000000; k++) 
	{
		uint64_t magic = random_uint64_fewbits();
		if(std::popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

		for(int i = 0; i < 4096; i++) used[i] = 0ULL;

		int i, fail;
		for(i = 0, fail = 0; !fail && i < (1 << n); i++)
		{
			int j = apply_magic(block[i], magic, n);
			
			if(used[j] == 0ULL) used[j] = move[i];
			else if(used[j] != move[i]) fail = 1;
		}

		if(!fail) 
			return magic;
	}
	std::cout << "Failed for sq = " << sq << " bishop = " << bishop << "\n";
	return 0ULL;
}



int main(int argc, char* argv[])
{
	std::string out_path = "../src/magic.h";
	if(argc > 1)
		out_path = argv[1];
	
	std::ofstream fout(out_path);

	fout << "#pragma once\n\n";

	fout << "const uint64_t RMagic[64] = {\n";
	for(int sq = 0; sq < 64; sq++)
	{
		std::cout << "\rcalculating rook magic for sq = " << sq << std::flush;
		fout << find_magic(sq, 0) << ",\n";
	}
	std::cout << "\rrook magic done!                  " << std::endl;
	fout << "};\n\n";

	fout << "const uint64_t BMagic[64] = {\n";
	for(int sq = 0; sq < 64; sq++)
	{
		std::cout << "\rcalculating bishop magic for sq = " << sq << std::flush;
		fout << find_magic(sq, 1) << ",\n";
	}
	std::cout << "\rbishop magic done!                  " << std::endl;
	fout << "};\n\n";

	fout.close();
	return 0;
}
