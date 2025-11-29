#ifndef __PIECE__
#define __PIECE__

#include "move.h"
#include <vector>

struct board_t;

struct piece_t {
	bool color; // Convention: False = White, True = Balck
	char symbol; // P (pawn), K (king), Q (queen), N (knight), B (bishop), R (rook)
	
	virtual std::vector<move_t> get_available_moves(board_t* board, int rank, int file) = 0;
};

struct king_t : piece_t {
	king_t(bool color){ this->color = color; this->symbol = 'K'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct queen_t : piece_t {
	queen_t(bool color){ this->color = color; this->symbol = 'Q'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct rook_t : piece_t {
	rook_t(bool color){ this->color = color; this->symbol = 'R'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct bishop_t : piece_t {
	bishop_t(bool color){ this->color = color; this->symbol = 'B'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct knight_t : piece_t {
	knight_t(bool color){ this->color = color; this->symbol = 'N'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct pawn_t : piece_t {
	pawn_t(bool color){ this->color = color; this->symbol = 'P'; }
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

#endif
