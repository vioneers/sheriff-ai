#ifndef __PIECE__
#define __PIECE__

#include "move.h"
#include <vector>

struct board_t;

enum pieces{
	// White pieces
    PAWN_W, ROOK_W, KNIGHT_W, BISHOP_W, QUEEN_W, KING_W,

    // Black pieces
    PAWN_B, ROOK_B, KNIGHT_B, BISHOP_B, QUEEN_B, KING_B,

    PIECE_COUNT // = 12 
};

struct piece_t {
	bool color; // Convention: False = White, True = Balck
	char symbol; // P (pawn), K (king), Q (queen), N (knight), B (bishop), R (rook)
	
	piece_t(bool color, char symbol)
        : color(color), symbol(symbol) {}
	
	virtual std::vector<move_t> get_available_moves(board_t* board, int rank, int file) = 0;
};

struct king_t : piece_t {
	king_t(bool color) : piece_t(color, 'K') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct queen_t : piece_t {
	queen_t(bool color) : piece_t(color, 'Q') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct rook_t : piece_t {
	rook_t(bool color) : piece_t(color, 'R') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct bishop_t : piece_t {
	bishop_t(bool color) : piece_t(color, 'B') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct knight_t : piece_t {
	knight_t(bool color) : piece_t(color, 'N') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

struct pawn_t : piece_t {
	pawn_t(bool color) : piece_t(color, 'P') {}
	std::vector<move_t> get_available_moves(board_t* board, int rank, int file);
};

#endif
