#include "zobrist.h"
#include <random>

uint64_t Z_PSQ[12][64];
uint64_t Z_TURN; 
uint64_t Z_CASTLE[16];
uint64_t Z_EPFILE[9];

uint64_t random64(std::mt19937_64 &rng){ // to generate random numbers
    return rng();
}

void init_zobrist(uint64_t seed){
    std::mt19937_64 rng(seed);

    for (int piece = 0; piece < 12; piece++) // 12 types of pieces
    {
        for (int square = 0; square < 64; square++) // 64 squares on the board
            Z_PSQ[piece][square] = random64(rng);
    }
    // Turn (black or white)
    Z_TURN = random64(rng);

    // Castles
    for (int i = 0; i < 16; i++)
        Z_CASTLE[i] = random64(rng);
    
    // En passant (one extra file for the no en passant case) 
    for (int i = 0; i < 9; i++)
        Z_EPFILE[i] = random64(rng);
}
