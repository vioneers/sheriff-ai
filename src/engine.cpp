#include <vector>
#include <limits>
#include <algorithm>

#include "engine.h"
#include "evaluationbar.h"

constexpr int INF = std::numeric_limits<int>::max();
constexpr int MATE_SCORE = 1000000;

static int mv_piece_value(char s){
    switch(s){
        case 'P': return 100;
        case 'N': return 300;
        case 'B': return 320;
        case 'R': return 500;
        case 'Q': return 900;
        case 'K': return 20000;
        default: return 0;
    }
}

static int move_order_score(const move_t& m, board_t& b){
    piece_t* mover = b.get_piece(m.from_rank, m.from_file);
    piece_t* target = b.get_piece(m.to_rank, m.to_file);
    int score = 0;

    // Captures (including en passant) using MVV-LVA style
    bool is_capture = target != nullptr;
    if (!is_capture && mover && mover->symbol == 'P' && m.to_file != m.from_file){
        params_t params = b.param_stack.back();
        if (params.ep_rank == m.to_rank && params.ep_file == m.to_file){
            is_capture = true;
            target = b.get_piece(m.from_rank, m.to_file); // captured pawn position
        }
    }
    if (is_capture && mover){
        int victim = target ? mv_piece_value(target->symbol) : mv_piece_value('P');
        int attacker = mv_piece_value(mover->symbol);
        score += 100000 + victim * 10 - attacker;
    }

    // Promotions to the front
    if (m.promotion){
        char promo_sym = std::toupper(m.promotion) == 'K' ? 'N' : std::toupper(m.promotion);
        score += 90000 + mv_piece_value(promo_sym);
    }

    return score;
}

engine_t::engine_t(std::vector<move_t> move_hist)
    : board_state(move_hist), 
	  best_move("e2e4"), // placeholder value 
      best_move_valid(false) {};

int engine_t::evaluate(){
	return evaluate_board(&board_state);
}

int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root){
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up || depth_left == 0) 
        return evaluate();

    int best = -INF;

    std::vector <move_t> legal = board_state.get_legal_moves(); 
    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, board_state) > move_order_score(b, board_state);
    });
    
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
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
        
    if (time_up || depth_left == 0) 
        return -evaluate();

    int best = INF;

    std::vector <move_t> legal = board_state.get_legal_moves(); 
    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, board_state) > move_order_score(b, board_state);
    });

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
