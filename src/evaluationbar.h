// #pragma once

// #include "board.h"

// // ---------------------------------------------------------
// //  EXISTING EVALUATION FUNCTIONS
// // ---------------------------------------------------------
// int evaluate_material_and_position(board_t* board);
// int evaluate_pawn_structure(board_t* board);
// int evaluate_mobility(board_t* board);
// int evaluate_king_safety(board_t* board); // keep for future use
// int evaluate_board(board_t* board);
// void print_evaluation_bar(board_t* board);

// // ---------------------------------------------------------
// //  NEW: EARLY GAME / ENDGAME HELPERS
// // ---------------------------------------------------------
// int evaluate_early_game_bonus(board_t* board);  // early game bonuses (knights, bishops, central pawns)
// int evaluate_endgame_king(board_t* board);      // endgame king restriction & checkmate awareness
// double game_phase_factor(board_t* board);       // 0=endgame, 1=opening
#pragma once
struct board_t;
// Main evaluation
int evaluate_board(board_t& board);

// UI helper
void print_evaluation_bar(board_t& board);
