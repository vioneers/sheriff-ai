#include <vector>
#include <limits>
#include <algorithm>
#include <bit>

#ifdef DEBUG
#include <iostream>
#endif

#include "engine.h"
#include "evaluationbar.h"
#include "zobrist.h"

constexpr int INF = std::numeric_limits<int>::max();
constexpr int LMR_FULL_MOVES = 3;
constexpr int LMR_MIN_DEPTH = 3;
constexpr int LMR_DEEPER_MOVES = 6;
constexpr int LMR_DEEPER_DEPTH = 5;
constexpr int NMP_MIN_DEPTH = 3;
constexpr int NMP_REDUCTION = 2;
constexpr int DELTA_MARGIN = 120;
constexpr int CHECK_EXTENSION = 1;
constexpr int HISTORY_MAX = 400000;

static int mv_piece_value(PieceType s){
    switch(s){
        case PAWN: return 100;
        case KNIGHT: return 320;
        case BISHOP: return 330;
        case ROOK: return 500;
        case QUEEN: return 900;
        case KING: return 20000;
        default: return 0;
    }
}

static int promo_piece_value(int flag){
    switch (flag) {
        case PROMO_N:
        case PROMO_N_CAP:
            return mv_piece_value(KNIGHT);
        case PROMO_B:
        case PROMO_B_CAP:
            return mv_piece_value(BISHOP);
        case PROMO_R:
        case PROMO_R_CAP:
            return mv_piece_value(ROOK);
        case PROMO_Q:
        case PROMO_Q_CAP:
            return mv_piece_value(QUEEN);
        default:
            return 0;
    }
}

static int delta_prune_gain(const move_t& m, const board_t& board){
    int gain = 0;
    int flag = m.flag();
    bool is_capture = (flag & CAPTURE) != 0;
    if (is_capture) {
        PieceType target = board.mailbox[m.to()];
        if (flag == EP_CAPTURE)
            target = PAWN;
        gain += mv_piece_value(target);
    }
    int promo_val = promo_piece_value(flag);
    if (promo_val > 0)
        gain += promo_val - mv_piece_value(PAWN);
    return gain;
}

static int popcount_bb(Bitboard bb){
    return std::popcount(bb);
}

static inline bool null_move_allowed(const board_t& b){
    int pawns = popcount_bb(b.pieces[PAWN] & b.occupancy[BOTH]);
    int majors = popcount_bb((b.pieces[ROOK] | b.pieces[QUEEN]) & b.occupancy[BOTH]);
    int minors = popcount_bb((b.pieces[KNIGHT] | b.pieces[BISHOP]) & b.occupancy[BOTH]);
    
    return !(pawns == 0 && majors == 0 && minors <= 2);
}

static inline bool lmr_allowed(bool is_quiet, bool is_root, bool in_check, bool is_check, int depth_left, int ply, double phase) {
    bool is_endgame = phase < 0.25;
    bool do_lmr = !is_root && is_quiet && !in_check && !is_check && !is_endgame && depth_left >= LMR_MIN_DEPTH && ply >= 2;

    return do_lmr;
}

void engine_t::update_history(int from, int to, int depth) {
    int bonus = depth * depth;
    if (history_table[from][to] < HISTORY_MAX - bonus)
        history_table[from][to] += bonus;
}

void engine_t::update_killers(int ply, uint16_t move_data) {
    if (ply >= 0 && ply < MAX_PLY) {
        if (killer_moves[ply][0] != move_data) {
            killer_moves[ply][1] = killer_moves[ply][0];
            killer_moves[ply][0] = move_data;
        }
    }
}

#ifdef DEBUG
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

int engine_t::move_order_score(const move_t& m, int ply, bool winning, bool almost_50_move){
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

    bool is_pawn = (mover == PAWN);
    bool found_reset = is_capture || is_pawn; // found a capture or pawn move, so 50 move timer would be reset
    
    if (winning && almost_50_move && found_reset){
        score += 6000; // bonus for reset
        if (is_capture) // bigger extra bonus for capture
            score += 2000; 
        if (is_pawn) // extra bonus for pawn (because we can also have pawn capture) 
            score += 1000;
    }
    // if we are losing, we'd prefer the draw so no bonus to do capture/pawn move
    return score;
}

void engine_t::score_moves(std::vector<move_t> &moves, int ply) {
    bool almost_50_move = (board.plies_since_irrev() >= 70);
    int eval = evaluate_board(board);
    // haven't made the move yet so don't use ~turn for Us 
    Color Us = board.history.back().turn; // from what perspective we evaluate

    int sign = Us == WHITE ? 1 : -1; 
    bool winning = (sign * eval > 200); 

    for (auto& move : moves)
        move.score = move_order_score(move, ply, winning, almost_50_move);
}

engine_t::engine_t(const std::vector<move_t> &move_hist){
    // initializations of zobrist key and opening book lookup table moved to engine constructor
    init_zobrist();
    board = board_t(move_hist);
    root_best_move = move_t{}; // initialize as null move
    // openings.init_lookup_table();
};

int engine_t::evaluate(){
    int eval = evaluate_board(board);
    Color Us = ~board.history.back().turn; // from what perspective we evaluate

    int sign = Us == WHITE ? 1 : -1; 

    // if in winning position => penalize repetitions
    // if White: check eval > 200
    // if Black: check -eval > 200, so eval < -200
    // changed to have abs(eval) - margin because mate is +/-(MATE_SCORE - ply)
    if (sign * eval > 200 && board.is_repetition(2) && !(std::abs(eval) >= MATE_SCORE - 1000))
        eval -= sign * 50;
        
    return eval;
    
}

int engine_t::quiesenceSearchMax(int alpha, int beta, int ply){
    using clock = std::chrono::steady_clock;
    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
    
    if(time_up)
        return 0;

    if(board.is_repetition(3))
        return 0;
    
    // check for 50 move rule draw
    if (board.plies_since_irrev() >= 100) 
        return 0;

    int stand_pat = evaluate();

    if (stand_pat >= beta)
        return beta;

    int delta_alpha = alpha;

    if (stand_pat > alpha)
        alpha = stand_pat;

    std::vector<move_t> legal; 
    board.get_legal_moves(legal); 
    
    bool is_check = board.in_check(board.history.back().turn);
    if (!is_check){
        // keep only captures and promotions if not in check
        // if in check, keep all legal moves
        for (auto it = legal.begin(); it != legal.end(); ){ // increasing it by case - see below
            bool is_capture = (it->flag() & CAPTURE) != 0;
            bool is_promo = (it->flag() & PROMO_N) != 0;
            if (!(is_capture || is_promo))
                it = legal.erase(it); // remove this move and go to the next one 
            else   
                ++it;
        }
    }

    // score the moves using move_order_score
    score_moves(legal, ply);
    // sort by move_order_score
    std::stable_sort(legal.begin(), legal.end());

    if (legal.empty() && is_check)
        return -MATE_SCORE + ply;

    int best = alpha;
    for (auto &move : legal){
        if (!is_check) {
            int gain = delta_prune_gain(move, board);
            if (stand_pat + gain + DELTA_MARGIN <= delta_alpha)
                continue;
        }
        board.make_move(move);
        int score = quiesenceSearchMin(best, beta, ply + 1);
        board.undo_move(move);

        if (time_up)
            return 0;

        if (score >= beta)
            return beta;
        if (score > best)
            best = score;
    }
    return best;
}

int engine_t::quiesenceSearchMin(int alpha, int beta, int ply){
    using clock = std::chrono::steady_clock;
    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;
    
    if(time_up)
        return 0;

    if(board.is_repetition(3))
        return 0;

    // check for 50 move rule draw
    if (board.plies_since_irrev() >= 100) 
        return 0;

    int stand_pat = evaluate();

    if (stand_pat <= alpha)
        return alpha;

    int delta_beta = beta;

    if (stand_pat < beta)
        beta = stand_pat;

    std::vector<move_t> legal; 
    board.get_legal_moves(legal); 

    bool is_check = board.in_check(board.history.back().turn);
    if (!is_check){
        // keep only captures and promotions if not in check
        // if in check, keep all legal moves
        for (auto it = legal.begin(); it != legal.end(); ){ // increasing it by case - see below
            bool is_capture = (it->flag() & CAPTURE) != 0;
            bool is_promo = (it->flag() & PROMO_N) != 0;
            if (!(is_capture || is_promo))
                it = legal.erase(it); // remove this move and go to the next one 
            else   
                ++it;
        }
    }

    // score the moves using move_order_score
    score_moves(legal, ply);
    // sort by move_order_score
    std::stable_sort(legal.begin(), legal.end());

    if (legal.empty() && is_check)
        return MATE_SCORE - ply;

    int best = beta;
    for (auto &move : legal){
        if (!is_check) {
            int gain = delta_prune_gain(move, board);
            if (stand_pat - gain - DELTA_MARGIN >= delta_beta)
                continue;
        }
        board.make_move(move);
        int score = quiesenceSearchMax(alpha, best, ply + 1);
        board.undo_move(move);

        if (time_up)
            return 0;

        if (score <= alpha)
            return alpha;
        if (score < best)
            best = score;
    }
    return best;
}



int engine_t::alphaBetaMax(int alpha, int beta, int depth_left, bool is_root, int ply) {
    using clock = std::chrono::steady_clock;

    // check for timeout
    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif
        return 0; // we dont care about return, just abort
    }

    // parameters for this node (call of the function)
    move_t best_move{}; // initialy the null move
    int best = -INF;
    int alphaOrig = alpha; // store inital alpha (needed for TTstore)
    const uint64_t z_key = board.history.back().z_key;
    double board_phase = phase(board);

    if (is_root && ply == 0) {
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + (MAX_PLY * 2), 0);
        std::fill(&history_table[0][0], &history_table[0][0] + (64 * 64), 0);

#ifdef DEBUG
        std::fill(&pv_length[0], &pv_length[0] + MAX_PLY, 0);
        for (int i = 0; i < 3; ++i)
            root_lines[i].valid = false;
#endif
    }

    bool in_check = board.in_check(board.history.back().turn);
    int search_depth = depth_left + ((in_check && depth_left >= 1) ? CHECK_EXTENSION : 0);

    // check for threefold before timout just in case we can get a more acurate score
    if (board.is_repetition(3))
        return 0;

    // check for 50 move rule draw
    if (board.plies_since_irrev() >= 100) 
        return 0;

    // check TT before starting the search
    int ttScore = -INF;
    move_t ttMove{}; // initially ttMove is null

    if (checkTT(z_key, search_depth, ply, alpha, beta, ttScore, ttMove))
    {
#ifdef DEBUG
        TT_CUTOFFS++;
#endif
        return ttScore;
    }

    // if we already searched this position with another depth (from Iterative Deepening), 
    // start from the best move found then
    if(is_root && !root_best_move.is_null())
        ttMove = root_best_move;

    if (!is_root && !in_check && board_phase > 0.25 && search_depth >= NMP_MIN_DEPTH && null_move_allowed(board)) {
        int reduced_depth = search_depth - 1 - NMP_REDUCTION;
        if (reduced_depth < 0)
            reduced_depth = 0;
        if (board.make_null_move()){
            int score = alphaBetaMin(beta - 1, beta, reduced_depth, false, ply + 1);
            board.undo_null_move();
            
            if (time_up) {
#ifdef DEBUG
                pv_length[ply] = 0;
#endif
                return 0; // do not use score if time is up
            }
            
            if (score >= beta && std::abs(score) < MATE_SCORE - MAX_PLY)
            {
                // TTstore(z_key, beta, search_depth, ply, alphaOrig, beta, move_t{}); // store beta, not score
                return beta;
            }
        }
    }

    if (search_depth == 0) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif
        return quiesenceSearchMax(alpha, beta, ply);
    }

    std::vector <move_t> legal;
    board.get_legal_moves(legal);

    if (legal.empty()) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif
        if (is_root) // if we are at root and in stalemate we have lost 
            root_best_move = move_t{};

        best = in_check ? -MATE_SCORE + ply : 0; // checkmate or stalemate
        TTstore(z_key, best, search_depth, ply, alphaOrig, beta, best_move);
        return best;
    }

    score_moves(legal, ply);

    // sort using the overloaded operator of move_t which compares score
    std::stable_sort(legal.begin(), legal.end()); 

    // if TT has sugested a "best move" between the legal moves from this position, try that first
    if (!ttMove.is_null())
        for (move_t& move : legal)
            if (move.data == ttMove.data) {
                move_t tmp = legal[0];
                legal[0] = move;
                move = tmp;
                break;
            }

   
    /*if (is_root) {
        root_best_move = legal.front();
    }*/

    int move_index = 0;
    int full_depth = search_depth - 1;
    for (auto& move : legal) {

#ifdef DEBUG
        if (ply + 1 < MAX_PLY)
            pv_length[ply + 1] = 0;
#endif

        int score = 0;
        bool is_quiet = ((move.flag() & CAPTURE) == 0) && ((move.flag() & PROMO_N) == 0);
        bool is_pv = (move_index == 0);

        board.make_move(move);

        bool is_check = board.in_check(board.history.back().turn); // whether the move we made gave a check
        bool do_lmr = lmr_allowed(is_quiet, is_root, in_check, is_check, search_depth, ply, board_phase);

        if (do_lmr && !is_pv)
        {
            int reduction = (move_index >= LMR_DEEPER_MOVES && search_depth >= LMR_DEEPER_DEPTH) ? 2 : 1;
            int reduced_depth = full_depth - reduction;
            if (reduced_depth < 0)
                reduced_depth = 0;
            
            // try lmr
            score = alphaBetaMin(alpha, alpha + 1, reduced_depth, false, ply + 1);

            if (score > alpha)
                score = alphaBetaMin(alpha, alpha + 1, full_depth, false, ply + 1);
        } 
        else
        {
            // normal search with Principal Variation Search
            score = alphaBetaMin(alpha, is_pv ? beta : (alpha + 1), full_depth, false, ply + 1);
        }

        if (!is_pv && score > alpha && score < beta)
            score = alphaBetaMin(alpha, beta, full_depth, false, ply + 1);
        
        board.undo_move(move);

        if (time_up) {
        #ifdef DEBUG
            pv_length[ply] = 0;
        #endif
            return 0; // do not use scores if time is up
        }

#ifdef DEBUG
        if (is_root)
            update_root_lines(*this, move, score, ply, true);
#endif

        if (score > best) {
            if (is_root) {
                root_best_move = move;
            }
            best = score;
            best_move = move;
#ifdef DEBUG
            update_pv(*this, ply, move);
#endif
            if (score > alpha)
                alpha = score;
        }
        if (score >= beta) {
            if (is_quiet) {
                update_killers(ply, move.data);
                update_history(move.from(), move.to(), search_depth);
            }
            break; // just break is enough, makes flow simpler for TT
        }
        ++move_index;
    }

    if (std::abs(best) < MATE_SCORE - MAX_PLY){
        TTstore(z_key, best, search_depth, ply, alphaOrig, beta, best_move);
    }

    return best;
}



int engine_t::alphaBetaMin(int alpha, int beta, int depth_left, bool is_root, int ply) {
    using clock = std::chrono::steady_clock;

    // check for timeout
    if (!time_up && time_limit.count() > 0 && clock::now() - start_time > time_limit)
        time_up = true;

    if (time_up) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif
        return 0; // we dont care about return, just abort
    }

    // parameters for this node (call of the function)
    move_t best_move{}; // initialy the null move
    int best = INF;
    int betaOrig = beta; // store inital beta (needed for TTstore)
    const uint64_t z_key = board.history.back().z_key;
    double board_phase = phase(board);

    if (is_root && ply == 0) {
        std::fill(&killer_moves[0][0], &killer_moves[0][0] + (MAX_PLY * 2), 0);
        std::fill(&history_table[0][0], &history_table[0][0] + (64 * 64), 0);

#ifdef DEBUG
        std::fill(&pv_length[0], &pv_length[0] + MAX_PLY, 0);
        for (int i = 0; i < 3; ++i)
            root_lines[i].valid = false;
#endif
    }

    bool in_check = board.in_check(board.history.back().turn);
    int search_depth = depth_left + ((in_check && depth_left >= 1) ? CHECK_EXTENSION : 0);

    // check for threefold before timout just in case we can get a more acurate score
    if (board.is_repetition(3))
        return 0;

    // check for 50 move rule draw
    if (board.plies_since_irrev() >= 100) 
        return 0;

    // check TT before starting the search
    int ttScore = INF;
    move_t ttMove{}; // initially ttMove is null

    if (checkTT(z_key, search_depth, ply, alpha, beta, ttScore, ttMove))
    {
#ifdef DEBUG
        TT_CUTOFFS++;
#endif
        return ttScore;
    }

    // if we already searched this position with another depth (from Iterative Deepening), 
    // start from the best move found then
    if(is_root && !root_best_move.is_null())
        ttMove = root_best_move;

    if (!is_root && !in_check && board_phase > 0.25 && search_depth >= NMP_MIN_DEPTH && null_move_allowed(board)) {
        int reduced_depth = search_depth - 1 - NMP_REDUCTION;
        if (reduced_depth < 0)
            reduced_depth = 0;
        if (board.make_null_move()){
            int score = alphaBetaMax(alpha, alpha + 1, reduced_depth, false, ply + 1);
            board.undo_null_move();
            
            if (time_up) {
#ifdef DEBUG
                pv_length[ply] = 0;
#endif
                return 0; // do not use score if time is up
            }
            
            if (score <= alpha && std::abs(score) < MATE_SCORE - MAX_PLY)
            {
                // TTstore(z_key, alpha, search_depth, ply, alpha, betaOrig, move_t{}); 
                return alpha;
            }
        }
    }

    if (search_depth == 0) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif
        return quiesenceSearchMin(alpha, beta, ply);
    }

    std::vector <move_t> legal;
    board.get_legal_moves(legal);

    if (legal.empty()) {
#ifdef DEBUG
        pv_length[ply] = 0;
#endif

        if (is_root) // if we are at root and in stalemate we have lost 
            root_best_move = move_t{};

        best = in_check ? MATE_SCORE - ply : 0; // checkmate or stalemate
        TTstore(z_key, best, search_depth, ply, alpha, betaOrig, best_move);
        return best;
    }

    score_moves(legal, ply);

    // sort using the overloaded operator of move_t which compares score
    std::stable_sort(legal.begin(), legal.end());

    // if TT has sugested a "best move" between the legal moves from this position, try that first
    if (!ttMove.is_null())
        for (move_t& move : legal)
            if (move.data == ttMove.data) {
                move_t tmp = legal[0];
                legal[0] = move;
                move = tmp;
                break;
            }

    
    /*if (is_root) {
        root_best_move = legal.front();
    }*/

    int move_index = 0;
    int full_depth = search_depth - 1;
    for (auto& move : legal) {
#ifdef DEBUG
        if (ply + 1 < MAX_PLY)
            pv_length[ply + 1] = 0;
#endif

        int score = 0;
        bool is_quiet = ((move.flag() & CAPTURE) == 0) && ((move.flag() & PROMO_N) == 0);
        bool is_pv = (move_index == 0);

        board.make_move(move);

        bool is_check = board.in_check(board.history.back().turn); // whether the move we made gave a check
        bool do_lmr = lmr_allowed(is_quiet, is_root, in_check, is_check, search_depth, ply, board_phase);

        if (do_lmr && !is_pv)
        {
            int reduction = (move_index >= LMR_DEEPER_MOVES && search_depth >= LMR_DEEPER_DEPTH) ? 2 : 1;
            int reduced_depth = full_depth - reduction;
            if (reduced_depth < 0)
                reduced_depth = 0;

            // try lmr
            score = alphaBetaMax(beta - 1, beta, reduced_depth, false, ply + 1);
            
            if (score < beta)
                score = alphaBetaMax(beta - 1, beta, full_depth, false, ply + 1);
        }
        else
        {
            // normal search with Principal Variation Search
            score = alphaBetaMax(is_pv ? alpha : (beta - 1), beta, full_depth, false, ply + 1);
        }
        
        if (!is_pv && score < beta && score > alpha) {
            score = alphaBetaMax(alpha, beta, full_depth, false, ply + 1);
        }

        board.undo_move(move);

        if (time_up) {
        #ifdef DEBUG
            pv_length[ply] = 0;
        #endif
            return 0; // do not use scores if time is up
        }

#ifdef DEBUG
        if (is_root)
            update_root_lines(*this, move, score, ply, false);
#endif

        if (score < best) {
            if (is_root) {
                root_best_move = move;
            }
            best = score;
            best_move = move;
#ifdef DEBUG
            update_pv(*this, ply, move);
#endif
            if (score < beta)
                beta = score;
        }
        if (score <= alpha) {
            if (is_quiet) {
                update_killers(ply, move.data);
                update_history(move.from(), move.to(), search_depth);
            }
            break; // just break is enough, makes flow simpler for TT
        }
        ++move_index;
    }

    if (std::abs(best) < MATE_SCORE - MAX_PLY){
        TTstore(z_key, best, search_depth, ply, alpha, betaOrig, best_move);
    }

    return best;
}



move_t engine_t::get_strategy(){ // Play opening or get_best_move
    move_t best_move;
    bool found_opening = openings.probe(board,best_move);
    if (!found_opening)
        best_move = get_best_move();
    return best_move;
}

move_t engine_t::get_best_move(){ // with Iterative Deepening
    using clock = std::chrono::steady_clock;
    int depth = 1; 
    int prev_score = 0;
    bool has_prev = false;

    // Constants for aspiration windows
    constexpr int ASP_WINDOW_INITIAL = 30;
    constexpr int ASP_WINDOW_MAX = 500;
    constexpr int ASP_MIN_DEPTH = 4;  // Don't use aspiration below depth 4

    do{
        if (has_prev && std::abs(prev_score) >= MATE_SCORE - MAX_PLY) {
            std::fill(TT.begin(), TT.end(), TTentry{});
        }
        int score = 0;
        Color turn = board.history.back().turn;

        bool use_aspiration = has_prev 
                                && (std::abs(prev_score) < MATE_SCORE - 1000) 
                                && depth >= ASP_MIN_DEPTH;

        int alpha = -INF;
        int beta = INF;
        int window = ASP_WINDOW_INITIAL;

        if (use_aspiration) {
            alpha = prev_score - window;
            beta = prev_score + window;
        }

        while (true) {
            if (turn == WHITE)
                score = alphaBetaMax(alpha, beta, depth);
            else
                score = alphaBetaMin(alpha, beta, depth);

            if (time_up || !use_aspiration)
                break;

            if (score <= alpha){
                window *= 2;
                if (window >= ASP_WINDOW_MAX) {
                    alpha = -INF;
                    beta = INF;
                } 
                else {
                    alpha = prev_score - window;
                } 
            }
            else if (score >= beta){
                window *= 2;
                if (window >= ASP_WINDOW_MAX) {
                    alpha = -INF;
                    beta = INF;
                } else {
                    beta = prev_score + window;
                }
            }
            else{
                break;
            }
        }

        if(!time_up)
        {
            iterative_best_move = root_best_move;
            prev_score = score;
            has_prev = true;
#ifdef DEBUG
            std::cout << "finished depth " << depth << "\n";
            log_root_lines();
            std::cout << "\n";
            print_debug_info();
            std::cout << "\n";

            TT_PROBES = TT_HITS = TT_CUTOFFS = 0;
#endif
        }

        depth++;
        
    } while (!time_up && depth < DEFAULT_SEARCH_DEPTH);

    return iterative_best_move;
}

#ifdef DEBUG
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

#ifdef DEBUG
void engine_t::print_debug_info() const
{
    using clock = std::chrono::steady_clock;
    auto elapsed = clock::now() - start_time;

    std::cout << board.to_fen() << '\n';
    std::cout << '\n';
    std::cout << "running time : "
        << std::chrono::duration<double>(elapsed).count()
        << " s\n";
    std::cout << '\n';
    std::cout << "TT_PROBES : " << TT_PROBES << '\n';
    std::cout << "TT_HITS : " << TT_HITS << '\n';
    std::cout << "TT_CUTOFFS : " << TT_CUTOFFS << '\n';
}
#endif
#endif
