#include <vector>
#include <limits>
#include <algorithm>
#ifdef SHERIFF_DEBUG_PV
#include <iostream>
#endif

#include "engine.h"
#include "evaluationbar.h"
#include "zobrist.h"
#include "opening_book.h"

constexpr int INF = std::numeric_limits<int>::max();
constexpr int MATE_SCORE = 1000000;
constexpr int LMR_FULL_MOVES = 3;
constexpr int LMR_MIN_DEPTH = 3;
constexpr int LMR_DEEPER_MOVES = 6;
constexpr int LMR_DEEPER_DEPTH = 5;

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

#ifdef SHERIFF_DEBUG_PV
static void update_pv(engine_t& eng, int ply, const move_t& move){
    eng.pv_moves[ply][0] = move;
    int child_len = (ply + 1 < engine_t::MAX_PLY) ? eng.pv_length[ply + 1] : 0;
    if (child_len > engine_t::MAX_PLY - 1)
        child_len = engine_t::MAX_PLY - 1;
    for (int i = 0; i < child_len; ++i)
        eng.pv_moves[ply][i + 1] = eng.pv_moves[ply + 1][i];
    eng.pv_length[ply] = child_len + 1;
}

static bool is_better_score(int score, int best, bool maximizing){
    return maximizing ? (score > best) : (score < best);
}

static void update_root_lines(engine_t& eng, const move_t& move, int score, int ply, bool maximizing){
    engine_t::root_line_t line;
    line.move = move;
    line.score = score;
    int child_len = (ply + 1 < engine_t::MAX_PLY) ? eng.pv_length[ply + 1] : 0;
    if (child_len > engine_t::MAX_PLY - 1)
        child_len = engine_t::MAX_PLY - 1;
    line.pv_len = child_len + 1;
    line.pv[0] = move;
    for (int i = 0; i < child_len; ++i)
        line.pv[i + 1] = eng.pv_moves[ply + 1][i];
    line.valid = true;

    for (int i = 0; i < 3; ++i){
        if (!eng.root_lines[i].valid){
            eng.root_lines[i] = line;
            return;
        }
        if (is_better_score(score, eng.root_lines[i].score, maximizing)){
            for (int j = 2; j > i; --j)
                eng.root_lines[j] = eng.root_lines[j - 1];
            eng.root_lines[i] = line;
            return;
        }
    }
}
#endif

int engine_t::move_order_score(const move_t& m, int ply){
    PieceType mover = board.mailbox[m.from()];
    PieceType target = board.mailbox[m.to()];
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
    bool is_promotion = (m.flag() & PROMO_N) != 0;
    if (is_promotion){
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

    if (!is_capture && !is_promotion){
        if (ply >= 0 && ply < MAX_PLY){
            if (killer_moves[ply][0] == m.data)
                score += 80000;
            else if (killer_moves[ply][1] == m.data)
                score += 75000;
        }
        score += history_table[m.from()][m.to()];
    }

    return score;
}

engine_t::engine_t(std::vector<move_t> move_hist){
    // initializations of zobrist key and opening book lookup table moved to engine constructor
    init_zobrist();
    board = board_t(move_hist);
    best_move = move_t("e2e4"); // placeholder value 
    best_move_valid = false; 
    // OpeningBook openings; not used yet
    // openings.init_lookup_table();
};

// temporary
int engine_t::evaluate(){
	return evaluate_board(&board);
}

int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root, int ply){
    using clock = std::chrono::steady_clock;

    if (is_root && ply == 0){
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + (MAX_PLY * 2), 0);
        std::fill(&history_table[0][0], &history_table[0][0] + (64 * 64), 0);
#ifdef SHERIFF_DEBUG_PV
        std::fill(&pv_length[0], &pv_length[0] + MAX_PLY, 0);
        for (int i = 0; i < 3; ++i)
            root_lines[i].valid = false;
#endif
    }

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up || depth_left == 0) {
#ifdef SHERIFF_DEBUG_PV
        pv_length[ply] = 0;
#endif
        return evaluate();
    }
    if (board.is_threefold())
        return 0;

    int best = -INF;

    std::vector <move_t> legal;
    board.get_legal_moves(legal); 

    std::stable_sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, ply) > move_order_score(b, ply);
    });
    
    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
#ifdef SHERIFF_DEBUG_PV
        pv_length[ply] = 0;
#endif
        return board.in_check(board.history.back().turn) ? -MATE_SCORE + ply : 0; // checkmate or stalemate
    }

    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }

    bool in_check = board.in_check(board.history.back().turn);
    int move_index = 0;
    for (auto& move: legal){
        bool is_quiet = ((move.flag() & CAPTURE) == 0) && ((move.flag() & PROMO_N) == 0);
        bool do_lmr = !is_root && is_quiet && !in_check && depth_left >= LMR_MIN_DEPTH && move_index >= LMR_FULL_MOVES;
        int score = 0;

        if (do_lmr){
            int reduction = (move_index >= LMR_DEEPER_MOVES && depth_left >= LMR_DEEPER_DEPTH) ? 2 : 1;
            int reduced_depth = depth_left - 1 - reduction;
            if (reduced_depth < 0)
                reduced_depth = 0;
            board.make_move(move);
            score = alphaBetaMin(alpha, beta, reduced_depth, false, ply + 1);
            board.undo_move(move);
            if (score > alpha){
                board.make_move(move);
                score = alphaBetaMin(alpha, beta, depth_left - 1, false, ply + 1);
                board.undo_move(move);
            }
        } else {
            board.make_move(move);
            score = alphaBetaMin(alpha, beta, depth_left - 1, false, ply + 1);
            board.undo_move(move);
        }

#ifdef SHERIFF_DEBUG_PV
        if (is_root)
            update_root_lines(*this, move, score, ply, true);
#endif
        
        if (score > best){
            if (is_root){
                best_move = move;
                best_move_valid = true;
            }
            best = score;
#ifdef SHERIFF_DEBUG_PV
            update_pv(*this, ply, move);
#endif
            if (score > alpha)
                alpha = score; 
        }
        if (score >= beta){
            if (is_quiet){
                int from = move.from();
                int to = move.to();
                history_table[from][to] += depth_left * depth_left;
                if (ply >= 0 && ply < MAX_PLY){
                    if (killer_moves[ply][0] != move.data){
                        killer_moves[ply][1] = killer_moves[ply][0];
                        killer_moves[ply][0] = move.data;
                    }
                }
            }
            return score;
        }
        ++move_index;
    }
    return best;
}
int engine_t::alphaBetaMin(int alpha, int beta, int depth_left, bool is_root, int ply){
    using clock = std::chrono::steady_clock;

    if (is_root && ply == 0){
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + (MAX_PLY * 2), 0);
        std::fill(&history_table[0][0], &history_table[0][0] + (64 * 64), 0);
#ifdef SHERIFF_DEBUG_PV
        std::fill(&pv_length[0], &pv_length[0] + MAX_PLY, 0);
        for (int i = 0; i < 3; ++i)
            root_lines[i].valid = false;
#endif
    }

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
        
    if (time_up || depth_left == 0) {
#ifdef SHERIFF_DEBUG_PV
        pv_length[ply] = 0;
#endif
        return evaluate();
    }
    if (board.is_threefold())
        return 0;

    int best = INF;

    std::vector <move_t> legal;
    board.get_legal_moves(legal);

    std::stable_sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, ply) > move_order_score(b, ply);
    });

    if (legal.empty()){
        if (is_root)
            best_move_valid = false;
#ifdef SHERIFF_DEBUG_PV
        pv_length[ply] = 0;
#endif
        return board.in_check(board.history.back().turn) ? MATE_SCORE - ply : 0; // checkmate or stalemate
    }
    if (is_root){
        best_move = legal.front();
        best_move_valid = true;
    }
    bool in_check = board.in_check(board.history.back().turn);
    int move_index = 0;
    for (auto& move: legal){
        bool is_quiet = ((move.flag() & CAPTURE) == 0) && ((move.flag() & PROMO_N) == 0);
        bool do_lmr = !is_root && is_quiet && !in_check && depth_left >= LMR_MIN_DEPTH && move_index >= LMR_FULL_MOVES;
        int score = 0;

        if (do_lmr){
            int reduction = (move_index >= LMR_DEEPER_MOVES && depth_left >= LMR_DEEPER_DEPTH) ? 2 : 1;
            int reduced_depth = depth_left - 1 - reduction;
            if (reduced_depth < 0)
                reduced_depth = 0;
            board.make_move(move);
            score = alphaBetaMax(alpha, beta, reduced_depth, false, ply + 1);
            board.undo_move(move);
            if (score < beta){
                board.make_move(move);
                score = alphaBetaMax(alpha, beta, depth_left - 1, false, ply + 1);
                board.undo_move(move);
            }
        } else {
            board.make_move(move);
            score = alphaBetaMax (alpha, beta, depth_left - 1, false, ply + 1);
            board.undo_move(move);
        }

#ifdef SHERIFF_DEBUG_PV
        if (is_root)
            update_root_lines(*this, move, score, ply, false);
#endif
        
        if (score < best){
            if (is_root){
                best_move = move;
                best_move_valid = true;
            }
            best = score;
#ifdef SHERIFF_DEBUG_PV
            update_pv(*this, ply, move);
#endif
            if (score < beta)
                beta = score; 
        }
        if (score <= alpha){
            if (is_quiet){
                int from = move.from();
                int to = move.to();
                history_table[from][to] += depth_left * depth_left;
                if (ply >= 0 && ply < MAX_PLY){
                    if (killer_moves[ply][0] != move.data){
                        killer_moves[ply][1] = killer_moves[ply][0];
                        killer_moves[ply][0] = move.data;
                    }
                }
            }
            return score;
        }
        ++move_index;
    }
    return best;
}

#ifdef SHERIFF_DEBUG_PV
void engine_t::log_root_lines() const {
    for (int i = 0; i < 3; ++i){
        const auto& line = root_lines[i];
        if (!line.valid)
            continue;
        std::cout << "info string root" << (i + 1) << " score " << line.score << " pv";
        for (int j = 0; j < line.pv_len; ++j)
            std::cout << " " << line.pv[j].to_code();
        std::cout << '\n';
    }
}
#endif
