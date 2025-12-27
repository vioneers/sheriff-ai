#include <vector>
#include <limits>
#include <algorithm>

#include "engine.h"
#include "evaluationbar.h"

constexpr int INF = std::numeric_limits<int>::max();
constexpr int MATE_SCORE = 1000000;

static int mv_piece_value(PieceType s){
    switch(s){
        case PAWN: return 100;
        case KNIGHT: return 300;
        case BISHOP: return 320;
        case ROOK: return 500;
        case QUEEN: return 900;
        case KING: return 20000;
        default: return 0;
    }
}

static int move_order_score(const move_t& m, board_t& b){
    PieceType mover = b.mailbox[m.from()];
    PieceType target = b.mailbox[m.to()];
    int score = 0;

    // Captures (including en passant) using MVV-LVA style
    bool is_capture = (m.flag() & CAPTURE) != 0;
    if (m.flag() == EP_CAPTURE)
        target = PAWN;
    
    if (is_capture){
        int victim = mv_piece_value(target);
        int attacker = mv_piece_value(mover);
        score += 100000 + victim * 10 - attacker;
    }

    // Promotions to the front
    if ((m.flag() & PROMO_N) != 0){
        PieceType promo_piece;
        switch (m.flag()) {
            case PROMO_N:
            case PROMO_N_CAP:
                promo_piece = KNIGHT; break;
            case PROMO_B:
            case PROMO_B_CAP:
                promo_piece = BISHOP; break;
            case PROMO_R:
            case PROMO_R_CAP:
                promo_piece = ROOK; break;
            case PROMO_Q:
            case PROMO_Q_CAP:
                promo_piece = QUEEN; break;
        }
        score += 90000 + mv_piece_value(promo_piece);
    }

    return score;
}

engine_t::engine_t(std::vector<move_t> move_hist)
    : board(move_hist), 
	  best_move("e2e4"), // placeholder value 
      best_move_valid(false) {};

// temporary
int engine_t::evaluate(){
	return evaluate_board(&board);
}

int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root){
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up || depth_left == 0) 
        return evaluate();

    int best = -INF;

    std::vector <move_t> legal;
    Color turn = board.history.back().turn;
    board.get_legal_moves(legal, turn); 

    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, board) > move_order_score(b, board);
    });
    
    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
        return board.in_check(board.history.back().turn) ? -MATE_SCORE + depth_left : 0; // checkmate or stalemate
    }

    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }

    for (auto& move: legal){
        board.make_move(move);
        int score = alphaBetaMin(alpha, beta, depth_left - 1, false);
        board.undo_move(move);
        
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
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
        
    if (time_up || depth_left == 0) 
        return -evaluate();

    int best = INF;

    std::vector <move_t> legal;
    Color turn = board.history.back().turn;
    board.get_legal_moves(legal, turn);

    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, board) > move_order_score(b, board);
    });

    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
        return board.in_check(board.history.back().turn) ? MATE_SCORE - depth_left : 0; // checkmate or stalemate
    }
    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }
    for (auto& move: legal){
        board.make_move(move);
        int score = alphaBetaMax (alpha, beta, depth_left - 1, false);
        board.undo_move(move);
        
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
