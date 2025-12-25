#include "evaluationbar.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "piece.h"
#include "board.h"
#include "move.h"

// ---------------------------------------------------------
//  MATERIAL VALUES
// ---------------------------------------------------------
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

// ---------------------------------------------------------
//  PIECE-SQUARE TABLES
//  White aligned, mirrored for Black
// ---------------------------------------------------------
static const int pawn_table[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {50,50,50,50,50,50,50,50},
    {10,10,20,30,30,20,10,10},
    {5, 5,10,25,25,10, 5, 5},
    {0, 0, 0,20,20, 0, 0, 0},
    {5,-5,-10, 0, 0,-10,-5, 5},
    {5,10,10,-20,-20,10,10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

static const int knight_table[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishop_table[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
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
    {0,0,5,10,10,5,0,0}
};

static const int queen_table[8][8] = {
    {-20,-10,-10,-5,-5,-10,-10,-20},
    {-10,0,0,0,0,0,0,-10},
    {-10,0,5,5,5,5,0,-10},
    {-5,0,5,5,5,5,0,-5},
    {0,0,5,5,5,5,0,-5},
    {-10,0,5,5,5,5,0,-10},
    {-10,0,0,0,0,0,0,-10},
    {-20,-10,-10,-5,-5,-10,-10,-20}
};

// King safer in endgame → simplified version
static const int king_safety_table[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    {20, 20, 0, 0, 0, 0, 20,20},
    {30, 40, 10,0, 0,10,40,30}
};

// ---------------------------------------------------------
//  MATERIAL + PST Eval
// ---------------------------------------------------------
int evaluate_material_and_position(board_t* board) {
    int score = 0;
    for(int r = 0; r < 8; r++) {
        for(int f = 0; f < 8; f++) {
            piece_t* p = board->get_piece(r, f);
            if (!p) continue;

            int v = piece_value(p->symbol);
            int bonus = 0;

            switch(p->symbol) {
                case 'P': bonus = p->color ? pawn_table[7-r][f] : pawn_table[r][f]; break;
                case 'N': bonus = p->color ? knight_table[7-r][f] : knight_table[r][f]; break;
                case 'B': bonus = p->color ? bishop_table[7-r][f] : bishop_table[r][f]; break;
                case 'R': bonus = p->color ? rook_table[7-r][f] : rook_table[r][f]; break;
                case 'Q': bonus = p->color ? queen_table[7-r][f] : queen_table[r][f]; break;
                case 'K': bonus = p->color ? king_safety_table[7-r][f] : king_safety_table[r][f]; break;
            }

            score += (p->color ? -(v + bonus) : +(v + bonus));
        }
    }
    return score;
}

// ---------------------------------------------------------
//  MOBILITY EVALUATION
// ---------------------------------------------------------
static const int MOBILITY = 5;

int evaluate_mobility(board_t* board) {
    int score = 0;
    auto moves = board->get_legal_moves();

    for(const auto& m : moves) {
        piece_t* p = board->get_piece(m.from_rank, m.from_file);
        if (!p) continue;
        score += (p->color ? -MOBILITY : +MOBILITY);
    }
    return score;
}

// ---------------------------------------------------------
//  PAWN STRUCTURE: Isolated & Doubled penalties
// ---------------------------------------------------------
static const int ISOLATED_PAWN_PENALTY = 15;
static const int DOUBLED_PAWN_PENALTY  = 20;

int evaluate_pawn_structure(board_t* board) {
    int score = 0;
    int pawnsW[8]={0}, pawnsB[8]={0};

    for(int r=0;r<8;r++){
        for(int f=0;f<8;f++){
            piece_t* p=board->get_piece(r,f);
            if(!p || p->symbol!='P') continue;
            if(p->color) pawnsB[f]++; else pawnsW[f]++;
        }
    }
    for(int f=0;f<8;f++){
        if(pawnsW[f] >= 2) score -= DOUBLED_PAWN_PENALTY;
        if(pawnsB[f] >= 2) score += DOUBLED_PAWN_PENALTY;

        if(pawnsW[f] == 1 && f>0 && f<7 && pawnsW[f-1]==0 && pawnsW[f+1]==0)
            score -= ISOLATED_PAWN_PENALTY;
        if(pawnsB[f] == 1 && f>0 && f<7 && pawnsB[f-1]==0 && pawnsB[f+1]==0)
            score += ISOLATED_PAWN_PENALTY;
    }
    return score;
}

// ---------------------------------------------------------
//  MAIN EVAL
// ---------------------------------------------------------
int evaluate_board(board_t* board) {
    int score = 0;
    score += 1.2*evaluate_material_and_position(board);
    score += 0.5*evaluate_mobility(board);
    score += evaluate_pawn_structure(board);
    return score;
}

// // ---------------------------------------------------------
//  PRINT EVALUATION BAR
// ---------------------------------------------------------
void print_evaluation_bar(board_t* board) {
    int cp = evaluate_board(board);
    double eval = cp / 100.0;

    double scaled = 50 + (eval / 10.0) * 50.0;
    if (scaled < 0) scaled = 0;
    if (scaled > 100) scaled = 100;

    int barCount = static_cast<int>(scaled / 5);

    std::cout << "[";
    for(int i = 0; i < barCount; i++) std::cout << "*";
    for(int i = barCount; i < 20; i++) std::cout << "-";
    std::cout << "] "
              << (eval >= 0 ? "+" : "") << eval
              << (eval > 0 ? " white" : (eval < 0 ? " black" : " equal"))
              << " eval\n";
}
