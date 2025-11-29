#ifndef __PIECE__
#define __PIECE__

#include "move.h"

struct board_t;

struct piece_t {
	bool color; // False = White, True = Balck
	char symbol; // p, k, q, n, b, r
	
	virtual move_t* get_available_moves(board_t* board, size_t pos_x, size_t pos_y);
};

//TODO: struct king_t(piece_t) etc

#endif
