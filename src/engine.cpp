#include <vector>
#include <limits>

#include "engine.h"
#include "evaluationbar.h"

constexpr int INF = std::numeric_limits<int>::max();
constexpr int MATE_SCORE = 1000000;

engine_t::engine_t(std::vector<move_t> move_hist)
    : best_move("e2e4"), // placeholder value 
      best_move_valid(false),
	  pawn_w(false),
      rook_w(false),
      knight_w(false),
      bishop_w(false),
      queen_w(false),
      king_w(false),
      pawn_b(true),
      rook_b(true),
      knight_b(true),
      bishop_b(true),
      queen_b(true),
      king_b(true)
{
    pieces[PAWN_W]   = &pawn_w;
    pieces[ROOK_W]   = &rook_w;
    pieces[KNIGHT_W] = &knight_w;
    pieces[BISHOP_W] = &bishop_w;
    pieces[QUEEN_W]  = &queen_w;
    pieces[KING_W]   = &king_w;

    pieces[PAWN_B]   = &pawn_b;
    pieces[ROOK_B]   = &rook_b;
    pieces[KNIGHT_B] = &knight_b;
    pieces[BISHOP_B] = &bishop_b;
    pieces[QUEEN_B]  = &queen_b;
    pieces[KING_B]   = &king_b;

    board_state = board_t(move_hist, pieces);
}

int engine_t::evaluate(){
	return evaluate_board(&board_state);
}

int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root){
    if (depth_left == 0)
        return evaluate(); 
    int best = -INF;
    std::vector <move_t> legal = board_state.get_legal_moves(); 
    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
        return board_state.in_check(board_state.turn) ? -MATE_SCORE + depth_left : 0; // checkmate or stalemate
    }
    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }
    for (auto& move: legal){
        board_state.make_move(move);
        int score = alphaBetaMin (alpha, beta, depth_left - 1, false);
        board_state.undo_move(move);
        
        if (score >= best){
            if (is_root){
                best_move = move;
                best_move_valid = true;
            }
            best = score;
            if (score > alpha)
                alpha = score; 
        }
        if (score >= beta)
            return score;
    }
    return best;
}
int engine_t::alphaBetaMin(int alpha, int beta, int depth_left, bool is_root){
    if (depth_left == 0)
        return -evaluate(); 
    int best = INF;
    std::vector <move_t> legal = board_state.get_legal_moves(); 
    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
        return board_state.in_check(board_state.turn) ? MATE_SCORE - depth_left : 0; // checkmate or stalemate
    }
    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }
    for (auto& move: legal){
        board_state.make_move(move);
        int score = alphaBetaMax (alpha, beta, depth_left - 1, false);
        board_state.undo_move(move);
        
        if (score <= best){
            if (is_root){
                best_move = move;
                best_move_valid = true;
            }
            best = score;
            if (score < beta)
                beta = score; 
        }
        if (score <= alpha)
            return score;
    }
    return best;
}
