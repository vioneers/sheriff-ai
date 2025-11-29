#ifndef __BOARD__
#define __BOARD__

struct move_t;
struct piece_t;

struct board_t {
	piece_t* board[8][8];

	bool check_move(move_t* move);
	piece_t* get_piece(int rank, int file);

	bool check_in_between(int fr, int ff, int tr, int tf) const;
	bool square_attacked(int r, int f, bool by_color) const;
	bool in_check(bool color) const;
};

#endif
