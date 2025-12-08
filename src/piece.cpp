#include "board.h"

template<Color Us> 
void generate_pawn_moves(MoveList& list)
{
	constexpr Color Them = ~Us;
	constexpr int Up     = (Us == WHITE) ?  8 : -8;
    constexpr int Right  = (Us == WHITE) ?  9 : -7;
    constexpr int Left   = (Us == WHITE) ?  7 : -9;
	
	//TODO: finish this
	
}

template<Color Us> 
void generate_knight_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_king_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_queen_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_rook_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_bishop_moves(MoveList& list)
{
	// TODO:
}
