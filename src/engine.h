#pragma once

#include <chrono>
#include "board.h"

struct engine_t {
	board_t board_state; 	
    move_t best_move; 
    bool best_move_valid;

    // Timing (to ensure within 10 seconds)
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds time_limit{0};
    bool time_up = false;

	engine_t(std::vector <move_t> move_hist);

    // Inspiration for the Alpha-Beta algorihtm: https://www.chessprogramming.org
    // depth_left = depth left until stopping
    int evaluate(); 
    int alphaBetaMax(int alpha, int beta, int depth_left, bool is_root = true);
    int alphaBetaMin(int alpha, int beta, int depth_left, bool is_root = true);
};
