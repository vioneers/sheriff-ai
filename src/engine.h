#ifndef __ENGINE__
#define __ENGINE__

#include <chrono>
#include "board.h"
#include "piece.h"

struct engine_t {
    static const int MAX_DEPTH = 64;

	board_t board_state; 	
	std::array<piece_t*, PIECE_COUNT> pieces;
    move_t best_move; 
    bool best_move_valid;
    int search_depth = 0;

    // Timing (to ensure within 10 seconds)
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds time_limit{0};
    bool time_up = false;

    // Move-ordering heuristics
    move_t killer_moves[MAX_DEPTH][2];
    int history_table[64][64]{};

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

    // Inspiration for the Alpha-Beta algorihtm: https://www.chessprogramming.org
    // depth_left = depth left until stopping
    int evaluate(); 
    int alphaBetaMax(int alpha, int beta, int depth_left, bool is_root = true);
    int alphaBetaMin(int alpha, int beta, int depth_left, bool is_root = true);

    int move_order_score(const move_t& m, int ply);
};

#endif
