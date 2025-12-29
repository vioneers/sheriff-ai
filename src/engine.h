#pragma once

#include <chrono>
#include "board.h"

struct engine_t {
	board_t board; 	
    move_t best_move; 
    bool best_move_valid;

    static constexpr int MAX_PLY = 64;
    static constexpr int DEFAULT_SEARCH_DEPTH = 7;
    uint16_t killer_moves[MAX_PLY][2]{};
    int history_table[64][64]{};
#ifdef SHERIFF_DEBUG_PV
    move_t pv_moves[MAX_PLY][MAX_PLY]{};
    int pv_length[MAX_PLY]{};

    struct root_line_t {
        move_t move{0};
        int score{0};
        int pv_len{0};
        move_t pv[MAX_PLY]{};
        bool valid{false};
    };
    root_line_t root_lines[3]{};
#endif

    // Timing (to ensure within 10 seconds)
    std::chrono::steady_clock::time_point start_time;
    std::chrono::milliseconds time_limit{0};
    bool time_up = false;

	engine_t(std::vector <move_t> move_hist);

    // Inspiration for the Alpha-Beta algorihtm: https://www.chessprogramming.org
    // depth_left = depth left until stopping
    int evaluate(); 
    int alphaBetaMax(int alpha, int beta, int depth_left, bool is_root = true, int ply = 0);
    int alphaBetaMin(int alpha, int beta, int depth_left, bool is_root = true, int ply = 0);

    int move_order_score(const move_t& m, int ply);
#ifdef SHERIFF_DEBUG_PV
    void log_root_lines() const;
#endif
};
