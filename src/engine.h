#pragma once

#include <chrono>
#include "board.h"
#include "opening_book.h"

constexpr int MATE_SCORE = 1000000; // needed for TT

struct engine_t {
	board_t board; 	
    move_t root_best_move; // best move over the possibly unfinished search
    move_t iterative_best_move; // best move over a completed search up to a certain depth

    OpeningBook openings; 

    static constexpr int MAX_PLY = 64;
    static constexpr int DEFAULT_SEARCH_DEPTH = 1000; // increased because we have iterative deepening, so this is just an absolute max (will stop when out of time)
    uint16_t killer_moves[MAX_PLY][2]{};
    int history_table[64][64]{};

#ifdef DEBUG
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

	engine_t(const std::vector <move_t> &move_hist);

    // Inspiration for the Alpha-Beta algorihtm: https://www.chessprogramming.org
    // depth_left = depth left until stopping
    int evaluate(); 
    int quiesenceSearchMax(int alpha, int beta, int ply);
    int quiesenceSearchMin(int alpha, int beta, int ply);
    int alphaBetaMax(int alpha, int beta, int depth_left, bool is_root = true, int ply = 0);
    int alphaBetaMin(int alpha, int beta, int depth_left, bool is_root = true, int ply = 0);
    move_t get_strategy(); // Play opening or get_best_move
    move_t get_best_move(); // wrapper for AlphaBeta with Iterative Deepening

    int move_order_score(const move_t& m, int ply, bool winning, bool almost_50_move);
    
    // call move_order_score and set move.score for all move in moves
    void score_moves(std::vector<move_t> &moves, int ply);

#ifdef DEBUG
    void log_root_lines() const;
    void print_debug_info() const;
#endif

    // transposition table (TT)
    static constexpr int TT_SIZE = 20; // TT length = 2^TT_SIZE

#ifdef DEBUG
    int TT_PROBES = 0; // counter of how many calls we make to probe
    int TT_HITS = 0; // number of times we found the node we were looking for in TT when probing
    int TT_CUTOFFS = 0; // number of positions we stoped searching thanks to TT
#endif

    struct TTentry { // 16 bytes
        uint64_t key;              // Zobrist hash
        int score;                 // evaluation score
        uint8_t depth_left;        // depth node was searched to (1 byte to reduce size)
        NodeType node_type;        // the type of score EXACT, LOWERBOUND, UPPERBOUND
        move_t bestMove;           // best move found from this position
    
    };
    std::vector<TTentry> TT{1 << TT_SIZE};          // 16MB //TODO: find good table size
   
    // implemented in transpotition.cpp
    
    TTentry& TTprobe(uint64_t z_key); // return reference the slot in TT where z_key points to (may be epmty or not contain the right board), always check the full z_key of the returned element
    
    void TTstore(
        uint64_t key,
        int score,
        int depth_left,
        int ply,                  // only used to normalize the mate score
        int alpha,                  // !!! use original bound that was given when calling alphaBetaMax()
        int beta,                   // !!! use original bound that was given when calling alphaBetaMin()
        move_t bestMove);           // store the current board state 
    
    bool checkTT(                   // check if we can use info from the TT to end the search (return TRUE if yes)
        uint64_t key,
        int depth_left,             // only used to normalize the mate score
        int ply,
        int alpha,
        int beta,
        int& outScore,
        move_t& outMove
    );
};
