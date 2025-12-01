#ifndef __ENGINE__
#define __ENGINE__

#include "board.h"
#include "piece.h"

enum pieces{
	// White pieces
    PAWN_W, ROOK_W, KNIGHT_W, BISHOP_W, QUEEN_W, KING_W,

    // Black pieces
    PAWN_B, ROOK_B, KNIGHT_B, BISHOP_B, QUEEN_B, KING_B,

    PIECE_COUNT // = 12 
};

struct engine_t {
	board_t board_state; 	
	std::array<piece_t*, PIECE_COUNT> pieces;

    // Piece objects
    pawn_t   pawn_w;
    rook_t   rook_w;
    knight_t knight_w;
    bishop_t bishop_w;
    queen_t  queen_w;
    king_t   king_w;

    pawn_t   pawn_b;
    rook_t   rook_b;
    knight_t knight_b;
    bishop_t bishop_b;
    queen_t  queen_b;
    king_t   king_b;

	engine_t(std::vector <move_t> move_hist);
};

#endif
