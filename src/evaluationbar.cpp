#include "evaluationbar.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "board.h"
#include "move.h"

// ---------------------------------------------------------
//  MATERIAL VALUES
// ---------------------------------------------------------
static int piece_value(int piece_type) {
    switch(piece_type) {
        case PAWN: return 100; 
        case KNIGHT: return 320; 
        case BISHOP: return 330; 
        case ROOK: return 500; 
        case QUEEN: return 900; 
        case KING: return 20000; 
        default: return 0;
    }
}

static int mirror_sq(int sq){
    return sq^56;
}

// ---------------------------------------------------------
//  PIECE-SQUARE TABLES
//  White aligned, mirrored for Black
// ---------------------------------------------------------
static const int pst[6][64] = {
    // P
    {0, 0, 0, 0, 0, 0, 0, 0,
     50,50,50,50,50,50,50,50,
     10,10,20,30,30,20,10,10,
     5, 5,10,25,25,10, 5, 5,
     0, 0, 0,20,20, 0, 0, 0,
     5,-5,-10, 0, 0,-10,-5, 5,
     5,10,10,-20,-20,10,10, 5,
     0, 0, 0, 0, 0, 0, 0, 0
    }, 
    // N 
    {-50,-40,-30,-30,-30,-30,-40,-50,
     -40,-20,  0,  0,  0,  0,-20,-40,
     -30,  0, 10, 15, 15, 10,  0,-30,
     -30,  5, 15, 20, 20, 15,  5,-30,
     -30,  0, 15, 20, 20, 15,  0,-30,
     -30,  5, 10, 15, 15, 10,  5,-30,
     -40,-20,  0,  5,  5,  0,-20,-40,
     -50,-40,-30,-30,-30,-30,-40,-50
    },
    // B
    {-20,-10,-10,-10,-10,-10,-10,-20,
     -10,  0,  0,  0,  0,  0,  0,-10,
     -10,  0,  5, 10, 10,  5,  0,-10,
     -10,  5,  5, 10, 10,  5,  5,-10,
     -10,  0, 10, 10, 10, 10,  0,-10,
     -10, 10, 10, 10, 10, 10, 10,-10,
     -10,  5,  0,  0,  0,  0,  5,-10,
     -20,-10,-10,-10,-10,-10,-10,-20
    },
    // R
    {0,0,0,0,0,0,0,0,
     5,10,10,10,10,10,10,5,
     -5,0,0,0,0,0,0,-5,
     -5,0,0,0,0,0,0,-5,
     -5,0,0,0,0,0,0,-5,
     -5,0,0,0,0,0,0,-5,
     -5,0,0,0,0,0,0,-5,
     0,0,5,10,10,5,0,0
    },
    // Q
    {-20,-10,-10,-5,-5,-10,-10,-20,
     -10,0,0,0,0,0,0,-10,
     -10,0,5,5,5,5,0,-10,
     -5,0,5,5,5,5,0,-5,
     0,0,5,5,5,5,0,-5,
     -10,0,5,5,5,5,0,-10,
     -10,0,0,0,0,0,0,-10,
     -20,-10,-10,-5,-5,-10,-10,-20
    },
    // K (King safer in endgame → simplified version)
    {-30,-40,-40,-50,-50,-40,-40,-30,
     -30,-40,-40,-50,-50,-40,-40,-30,
     -30,-40,-40,-50,-50,-40,-40,-30,
     -30,-40,-40,-50,-50,-40,-40,-30,
     -20,-30,-30,-40,-40,-30,-30,-20,
     -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20, 0, 0, 0, 0, 20,20,
     30, 40, 10,0, 0,10,40,30
    }
}

// ---------------------------------------------------------
//  MATERIAL + PST Eval
// ---------------------------------------------------------
int evaluate_material_and_position(board_t* board) {
    int score = 0;
    for (int sq = 0; sq < 64; sq++){
        int piece = board->mailbox[sq];
        if (piece == NONE) // no piece
            continue;
        bool is_black = (piece < 0); // negative => black; positive => white
        piece = std::abs(piece);
        int piece_val = piece_value(piece);
        int bonus = pst[piece - 1][is_black ? mirror_sq(sq) : sq];
        score += is_black ? -(v + bonus) : +(v + bonus);
    }
    return score;
}

// ---------------------------------------------------------
//  MOBILITY EVALUATION
// ---------------------------------------------------------
static const int MOBILITY = 5;

int evaluate_mobility(board_t* board) {
    std::vector<move_t> moves;
    board->get_legal_moves(moves, board->turn);

    int mobility = static_cast<int>(moves.size());

    return (board->turn == WHITE ? +mobility : -mobility) * MOBILITY;
}

// ---------------------------------------------------------
//  PAWN STRUCTURE: Isolated & Doubled penalties
// ---------------------------------------------------------
static const int ISOLATED_PAWN_PENALTY = 15;
static const int DOUBLED_PAWN_PENALTY  = 20;

int evaluate_pawn_structure(board_t* board) {
    int score = 0;
    int pawnsW[8]={0}, pawnsB[8]={0};

    for (int sq = 0; sq < 64; sq++){
        int piece = board->mailbox[sq];
        if (std::abs(piece) != PAWN) 
            continue;
        bool is_black = (piece < 0); // negative => black; positive => white
        int file = sq & 7; // sq % 8
        if(is_black)
            pawnsB[file]++;
        else
            pawnsW[file]++;
    }
    // negative points for white, positive points for black
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
