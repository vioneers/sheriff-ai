#ifndef __EVALUATIONBAR__
#define __EVALUATIONBAR__

#include "board.h"

// Evaluation functions
int evaluate_material_and_position(board_t* board);
int evaluate_pawn_structure(board_t* board);
int evaluate_mobility(board_t* board);
int evaluate_king_safety(board_t* board);
int evaluate_board(board_t* board);
void print_evaluation_bar(board_t* board);

#endif // 
