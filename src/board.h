#ifndef __BOARD__
#define __BOARD__

#include <piece.h>

struct board_t {
	piece_t* board[8][8];
	bool check_move(move_t* move);
};

#endif
