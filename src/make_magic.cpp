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

// prefer generating masks with few 1 bits for efficiency
uint64_t random_uint64_fewbits() {
  return random_uint64() & random_uint64() & random_uint64();
}

// find magic number for given square by trial and error
uint64_t find_magic(int sq, bool bishop)
{
	Bitboard mask = bishop? BishopMask[sq] : RookMask[sq];
	int n = std::popcount(mask); // get number of 1 bits in mask
	
	// generate all blocker configurations and the positions we can move to for each
	Bitboard block[4096], attack[4096], used[4096]; // we have at most 12 positions we are interested in, so 2^12 masks

	for (int i = 0; i < (1<<n); i++)
	{
		block[i] = index_to_blockers(i, n, mask);
		attack[i] = bishop? batt(sq, block[i]) : ratt(sq, block[i]);
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
			
			if(used[j] == 0ULL) used[j] = attack[i];
			else if(used[j] != attack[i]) fail = 1;
		}

		if(!fail) 
			return magic;
	}
	std::cout << "Failed for sq = " << sq << " bishop = " << bishop << "\n";
	return 0ULL;
}



int main(int argc, char* argv[])
{
	// pass the absolute output path as argument
	std::string out_path = "../src/magic.h";
	if(argc > 1)
		out_path = argv[1];
	
	std::ofstream fout(out_path);

	fout << "#pragma once\n\n";
	
	fout << "// Rook and Bishop shift tables\n\n";

	fout << "constexpr int RShift[64] = {";
	for(int sq = 0; sq < 64; sq++)
	{
		if(sq % 8 == 0)
			fout << "\n";
		fout << std::popcount(RookMask[sq]) << ", ";
	}
	fout << "}; \n\n";
	
	fout << "constexpr int BShift[64] = {";
	for(int sq = 0; sq < 64; sq++)
	{
		if(sq % 8 == 0)
			fout << "\n";
		fout << std::popcount(BishopMask[sq]) << ", ";
	}
	fout << "}; \n\n";

	fout << "// magic numbers tables\n\n";

	fout << "constexpr uint64_t RMagic[64] = {\n";
	for(int sq = 0; sq < 64; sq++)
	{
		std::cout << "\rcalculating rook magic for sq = " << sq << std::flush;
		fout << find_magic(sq, 0) << "ULL,\n";
	}
	std::cout << "\rrook magic done!                  " << std::endl;
	fout << "};\n\n";

	fout << "constexpr uint64_t BMagic[64] = {\n";
	for(int sq = 0; sq < 64; sq++)
	{
		std::cout << "\rcalculating bishop magic for sq = " << sq << std::flush;
		fout << find_magic(sq, 1) << "ULL,\n";
	}
	std::cout << "\rbishop magic done!                  " << std::endl;
	fout << "};\n\n";

	fout.close();
	return 0;
}
