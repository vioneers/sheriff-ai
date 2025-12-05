#include "evaluationbar.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "piece.h"

// --- Piece values ---
static int piece_value(char symbol) {
    switch(symbol) {
        case 'P': return 100;
        case 'N': return 320;
        case 'B': return 330;
        case 'R': return 500;
        case 'Q': return 900;
        case 'K': return 20000;
        default: return 0;
    }
}

// --- Piece-square tables for White (Black will be mirrored) ---
static const int pawn_table[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},
    {5, -5, -10, 0, 0, -10, -5, 5},
    {5, 10, 10, -20, -20, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

static const int knight_table[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,0,0,0,0,-20,-40},
    {-30,0,10,15,15,10,0,-30},
    {-30,5,15,20,20,15,5,-30},
    {-30,0,15,20,20,15,0,-30},
    {-30,5,10,15,15,10,5,-30},
    {-40,-20,0,5,5,0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishop_table[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,0,0,0,0,0,0,-10},
    {-10,0,5,10,10,5,0,-10},
    {-10,5,5,10,10,5,5,-10},
    {-10,0,10,10,10,10,0,-10},
    {-10,10,10,10,10,10,10,-10},
    {-10,5,0,0,0,0,5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int rook_table[8][8] = {
    {0,0,0,0,0,0,0,0},
    {5,10,10,10,10,10,10,5},
    {-5,0,0,0,0,0,0,-5},
    {-5,0,0,0,0,0,0,-5},
    {-5,0,0,0,0,0,0,-5},
    {-5,0,0,0,0,0,0,-5},
    {-5,0,0,0,0,0,0,-5},
    {0,0,0,5,5,0,0,0}
};

static const int queen_table[8][8] = {
    {-20,-10,-10,-5,-5,-10,-10,-20},
    {-10,0,0,0,0,0,0,-10},
    {-10,0,5,5,5,5,0,-10},
    {-5,0,5,5,5,5,0,-5},
    {0,0,5,5,5,5,0,-5},
    {-10,5,5,5,5,5,0,-10},
    {-10,0,5,0,0,0,0,-10},
    {-20,-10,-10,-5,-5,-10,-10,-20}
};

static const int king_table[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    {20,20,0,0,0,0,20,20},
    {20,30,10,0,0,10,30,20}
};

// --- Material + piece-square evaluation ---
int evaluate_material_and_position(board_t* board) {
    int score = 0;
    for(int r=0; r<8; r++) {
        for(int f=0; f<8; f++) {
            piece_t* p = board->get_piece(r,f);
            if(!p) continue;
            int pv = piece_value(p->symbol);
            int ps_bonus = 0;

            // Mirror table for Black
            switch(p->symbol) {
                case 'P': ps_bonus = p->color ? pawn_table[7-r][f] : pawn_table[r][f]; break;
                case 'N': ps_bonus = p->color ? knight_table[7-r][f] : knight_table[r][f]; break;
                case 'B': ps_bonus = p->color ? bishop_table[7-r][f] : bishop_table[r][f]; break;
                case 'R': ps_bonus = p->color ? rook_table[7-r][f] : rook_table[r][f]; break;
                case 'Q': ps_bonus = p->color ? queen_table[7-r][f] : queen_table[r][f]; break;
                case 'K': ps_bonus = p->color ? king_table[7-r][f] : king_table[r][f]; break;
            }

            score += (p->color ? -(pv + ps_bonus) : +(pv + ps_bonus));
        }
    }
    return score;
}

// --- Simplified evaluation (material + piece-square) ---
int evaluate_board(board_t* board) {
    return evaluate_material_and_position(board);
}

// --- Evaluation bar ---
void print_evaluation_bar(board_t* board){
    int cp = evaluate_board(board);
    double eval = cp/100.0;

    double scaled = 50 + (eval/10.0)*50.0;
    if(scaled<0) scaled=0;
    if(scaled>100) scaled=100;

    int barCount = static_cast<int>(scaled/5);
    std::cout<<"[";
    for(int i=0;i<barCount;i++) std::cout<<"*";
    for(int i=barCount;i<20;i++) std::cout<<"-";
    std::cout<<"] ";
    std::cout<<(eval>=0?"+":"")<<eval<<" (";
    if(eval>0) std::cout<<"white";
    else if(eval<0) std::cout<<"black";
    else std::cout<<"equal";
    std::cout<<") eval\n";
}
