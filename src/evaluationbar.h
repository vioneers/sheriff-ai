 #pragma once

 #include "board.h"

double phase(const board_t& board);

// Main evaluation
int evaluate_board(board_t& board);

// UI helper
void print_evaluation_bar(board_t& board);
