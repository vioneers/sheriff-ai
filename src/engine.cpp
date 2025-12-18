#include <vector>
#include <limits>
#include <algorithm>
#include <cctype>

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
    // Initialize killer table to placeholders
    for (int d = 0; d < MAX_DEPTH; ++d){
        killer_moves[d][0] = move_t();
        killer_moves[d][1] = move_t();
    }

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

int engine_t::move_order_score(const move_t& m, int ply){
    board_t& b = board_state;

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
    bool is_promotion = m.promotion != 0;
    if (is_promotion){
        char promo_sym = std::toupper(m.promotion) == 'K' ? 'N' : std::toupper(m.promotion);
        score += 90000 + mv_piece_value(promo_sym);
    }

    // Killer and history bonuses for quiet moves
    if (!is_capture && !is_promotion){
        if (killer_moves[ply][0] == m)
            score += 80000;
        else if (killer_moves[ply][1] == m)
            score += 75000;

        int from_sq = m.from_rank * 8 + m.from_file;
        int to_sq = m.to_rank * 8 + m.to_file;
        score += history_table[from_sq][to_sq];
    }

    return score;
}

static bool is_capture_move(const move_t& m, board_t& b){
    piece_t* mover = b.get_piece(m.from_rank, m.from_file);
    piece_t* target = b.get_piece(m.to_rank, m.to_file);
    bool is_capture = target != nullptr;
    if (!is_capture && mover && mover->symbol == 'P' && m.to_file != m.from_file){
        params_t params = b.param_stack.back();
        if (params.ep_rank == m.to_rank && params.ep_file == m.to_file){
            is_capture = true;
        }
    }
    return is_capture;
}

int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root){
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up || depth_left == 0) 
        return evaluate();

    if (is_root){
        search_depth = depth_left;
        for (int d = 0; d < MAX_DEPTH; ++d){
            killer_moves[d][0] = move_t();
            killer_moves[d][1] = move_t();
        }
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 64; ++j)
                history_table[i][j] = 0;
    }

    if (board_state.is_threefold())
        return 0;
    int best = -INF;

    std::vector <move_t> legal = board_state.get_legal_moves();
    int ply = search_depth - depth_left;
    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, ply) > move_order_score(b, ply);
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
        bool is_capture = is_capture_move(move, board_state);
        bool is_promotion = move.promotion != 0;

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
        if (score >= beta){
            if (!is_capture && !is_promotion){
                if (!(killer_moves[ply][0] == move)){
                    killer_moves[ply][1] = killer_moves[ply][0];
                    killer_moves[ply][0] = move;
                }
                int from_sq = move.from_rank * 8 + move.from_file;
                int to_sq = move.to_rank * 8 + move.to_file;
                history_table[from_sq][to_sq] += depth_left * depth_left;
            }
            return score;
        }
    }
    return best;
}
int engine_t::alphaBetaMin(int alpha, int beta, int depth_left, bool is_root){
    using clock = std::chrono::steady_clock;

    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
        
    if (time_up || depth_left == 0) 
        return evaluate();

    if (is_root){
        search_depth = depth_left;
        for (int d = 0; d < MAX_DEPTH; ++d){
            killer_moves[d][0] = move_t();
            killer_moves[d][1] = move_t();
        }
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 64; ++j)
                history_table[i][j] = 0;
    }

    if (board_state.is_threefold())
        return 0;
        
    int best = INF;

    std::vector <move_t> legal = board_state.get_legal_moves();
    int ply = search_depth - depth_left;
    std::sort(legal.begin(), legal.end(), [&](const move_t& a, const move_t& b){
        return move_order_score(a, ply) > move_order_score(b, ply);
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
        bool is_capture = is_capture_move(move, board_state);
        bool is_promotion = move.promotion != 0;

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
        if (score <= alpha){
            if (!is_capture && !is_promotion){
                if (!(killer_moves[ply][0] == move)){
                    killer_moves[ply][1] = killer_moves[ply][0];
                    killer_moves[ply][0] = move;
                }
                int from_sq = move.from_rank * 8 + move.from_file;
                int to_sq = move.to_rank * 8 + move.to_file;
                history_table[from_sq][to_sq] += depth_left * depth_left;
            }
            return score;
        }
    }
    return best;
}
