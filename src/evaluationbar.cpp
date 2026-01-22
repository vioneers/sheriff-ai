#include "evaluationbar.h"
#include "board.h"
#include "move.h"
#include "bitboard.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <bit>

static constexpr int PAWN_VALUE_MG   = 100;
static constexpr int PAWN_VALUE_EG   = 120;
static constexpr int KNIGHT_VALUE_MG = 320;
static constexpr int KNIGHT_VALUE_EG = 300;
static constexpr int BISHOP_VALUE_MG = 330;
static constexpr int BISHOP_VALUE_EG = 350;
static constexpr int ROOK_VALUE_MG   = 500;
static constexpr int ROOK_VALUE_EG   = 525;
static constexpr int QUEEN_VALUE_MG  = 900;
static constexpr int QUEEN_VALUE_EG  = 950;

static int piece_value_mg(int p) {
    switch (p) {
        case PAWN:   return PAWN_VALUE_MG;
        case KNIGHT: return KNIGHT_VALUE_MG;
        case BISHOP: return BISHOP_VALUE_MG;
        case ROOK:   return ROOK_VALUE_MG;
        case QUEEN:  return QUEEN_VALUE_MG;
        case KING:   return 0;
        default:     return 0;
    }
}

static int piece_value_eg(int p) {
    switch (p) {
        case PAWN:   return PAWN_VALUE_EG;
        case KNIGHT: return KNIGHT_VALUE_EG;
        case BISHOP: return BISHOP_VALUE_EG;
        case ROOK:   return ROOK_VALUE_EG;
        case QUEEN:  return QUEEN_VALUE_EG;
        case KING:   return 0;
        default:     return 0;
    }
}

static int piece_value(int p) {
    return piece_value_mg(p);
}

static int mirror_sq(int sq) { return sq ^ 56; }

static const int pst_mg[6][64] = {
    {
         0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
         5,  5, 10, 25, 25, 10,  5,  5,
         0,  0,  0, 20, 20,  0,  0,  0,
         5, -5,-10,  0,  0,-10, -5,  5,
         5, 10, 10,-20,-20, 10, 10,  5,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    },
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    },
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    },
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    }
};

static const int pst_eg[6][64] = {
    {
         0,  0,  0,  0,  0,  0,  0,  0,
        80, 80, 80, 80, 80, 80, 80, 80,
        50, 50, 50, 50, 50, 50, 50, 50,
        30, 30, 30, 30, 30, 30, 30, 30,
        20, 20, 20, 20, 20, 20, 20, 20,
        10, 10, 10, 10, 10, 10, 10, 10,
         5,  5,  5,  5,  5,  5,  5,  5,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    },
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  0, 10, 15, 15, 10,  0,-10,
        -10,  0, 10, 15, 15, 10,  0,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    },
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  0,  0,  0,  0,  0
    },
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5, 10, 10,  5,  0, -5,
         -5,  0,  5, 10, 10,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    }
};

static constexpr int PHASE_KNIGHT = 1;
static constexpr int PHASE_BISHOP = 1;
static constexpr int PHASE_ROOK   = 2;
static constexpr int PHASE_QUEEN  = 4;
static constexpr int PHASE_TOTAL  = 4*PHASE_KNIGHT + 4*PHASE_BISHOP + 4*PHASE_ROOK + 2*PHASE_QUEEN;

static int compute_phase(const board_t &board) {
    int phase = PHASE_TOTAL;
    
    phase -= PHASE_KNIGHT * std::popcount(board.pieces[KNIGHT]);
    phase -= PHASE_BISHOP * std::popcount(board.pieces[BISHOP]);
    phase -= PHASE_ROOK * std::popcount(board.pieces[ROOK]);
    phase -= PHASE_QUEEN * std::popcount(board.pieces[QUEEN]);
    
    phase = (PHASE_TOTAL - phase) * 256 / PHASE_TOTAL;
    
    return std::clamp(phase, 0, 256);
}

double phase(const board_t &board) {
    return compute_phase(board) / 256.0;
}

static inline int taper(int mg_score, int eg_score, int phase) {
    return (mg_score * phase + eg_score * (256 - phase)) / 256;
}

static void eval_material_pst(const board_t& board, int& mg_score, int& eg_score) {
    mg_score = 0;
    eg_score = 0;

    Bitboard pieces_bb = board.occupancy[BOTH];

    while (pieces_bb) {
        int sq = pop_lsb(pieces_bb);
        int p = board.mailbox[sq];
        if (p == NONE) continue;

        bool is_black = (board.occupancy[BLACK] >> sq) & 1ULL;
        int table_sq = is_black ? mirror_sq(sq) : sq;
        
        int mat_mg = piece_value_mg(p);
        int mat_eg = piece_value_eg(p);
        int pst_mg_val = pst_mg[p - 1][table_sq];
        int pst_eg_val = pst_eg[p - 1][table_sq];
        
        if (is_black) {
            mg_score -= mat_mg + pst_mg_val;
            eg_score -= mat_eg + pst_eg_val;
        } else {
            mg_score += mat_mg + pst_mg_val;
            eg_score += mat_eg + pst_eg_val;
        }
    }
}

static int mobility_for(const board_t& board, Color c) {
    Bitboard own = board.occupancy[c];
    Bitboard occ = board.occupancy[BOTH];
    int count = 0;

    Bitboard knights = board.pieces[KNIGHT] & own;
    while (knights) {
        int sq = pop_lsb(knights);
        count += std::popcount(KnightAttacks[sq] & ~own);
    }

    Bitboard bishops = board.pieces[BISHOP] & own;
    while (bishops) {
        int sq = pop_lsb(bishops);
        Bitboard blockers = occ & BishopMask[sq];
        int hash = apply_magic(blockers, BMagic[sq], BShift[sq]);
        Bitboard attacks = BishopAttacks[sq][hash];
        count += std::popcount(attacks & ~own);
    }

    Bitboard rooks = board.pieces[ROOK] & own;
    while (rooks) {
        int sq = pop_lsb(rooks);
        Bitboard blockers = occ & RookMask[sq];
        int hash = apply_magic(blockers, RMagic[sq], RShift[sq]);
        Bitboard attacks = RookAttacks[sq][hash];
        count += std::popcount(attacks & ~own);
    }

    Bitboard queens = board.pieces[QUEEN] & own;
    while (queens) {
        int sq = pop_lsb(queens);
        Bitboard rblockers = occ & RookMask[sq];
        int rhash = apply_magic(rblockers, RMagic[sq], RShift[sq]);
        Bitboard bblockers = occ & BishopMask[sq];
        int bhash = apply_magic(bblockers, BMagic[sq], BShift[sq]);
        Bitboard attacks = RookAttacks[sq][rhash] | BishopAttacks[sq][bhash];
        count += std::popcount(attacks & ~own);
    }
    return count;
}

static void eval_mobility(const board_t& board, int& mg_score, int& eg_score) {
    int w = mobility_for(board, WHITE);
    int b = mobility_for(board, BLACK);
    int diff = w - b;
    
    mg_score += diff * 5;
    eg_score += diff * 3;
}

static void eval_pawn_structure(const board_t& board, int& mg_score, int& eg_score) {
    int w[8] = {0}, b[8] = {0};
    
    for (int f = 0; f < 8; f++) {
        w[f] = std::popcount(board.occupancy[WHITE] & board.pieces[PAWN] & FileMask[f]);
        b[f] = std::popcount(board.occupancy[BLACK] & board.pieces[PAWN] & FileMask[f]);
    }

    int mg = 0, eg = 0;

    for (int f = 0; f < 8; f++) {
        if (w[f] >= 2) { mg -= 15 * (w[f] - 1); eg -= 25 * (w[f] - 1); }
        if (b[f] >= 2) { mg += 15 * (b[f] - 1); eg += 25 * (b[f] - 1); }
    }
    
    for (int f = 0; f < 8; f++) {
        bool w_isolated = false, b_isolated = false;
        
        if (f == 0) {
            w_isolated = (w[0] >= 1 && w[1] == 0);
            b_isolated = (b[0] >= 1 && b[1] == 0);
        } else if (f == 7) {
            w_isolated = (w[7] >= 1 && w[6] == 0);
            b_isolated = (b[7] >= 1 && b[6] == 0);
        } else {
            w_isolated = (w[f] >= 1 && w[f-1] == 0 && w[f+1] == 0);
            b_isolated = (b[f] >= 1 && b[f-1] == 0 && b[f+1] == 0);
        }
        
        if (w_isolated) { mg -= 12 * w[f]; eg -= 18 * w[f]; }
        if (b_isolated) { mg += 12 * b[f]; eg += 18 * b[f]; }
    }

    mg_score += mg;
    eg_score += eg;
}

static void eval_bishop_pair(const board_t& board, int& mg_score, int& eg_score) {
    int w_bishops = std::popcount(board.pieces[BISHOP] & board.occupancy[WHITE]);
    int b_bishops = std::popcount(board.pieces[BISHOP] & board.occupancy[BLACK]);
    
    if (w_bishops >= 2) { mg_score += 30; eg_score += 50; }
    if (b_bishops >= 2) { mg_score -= 30; eg_score -= 50; }
}

static void eval_rook_files(const board_t& board, int& mg_score, int& eg_score) {
    Bitboard w_pawns = board.pieces[PAWN] & board.occupancy[WHITE];
    Bitboard b_pawns = board.pieces[PAWN] & board.occupancy[BLACK];
    
    Bitboard w_rooks = board.pieces[ROOK] & board.occupancy[WHITE];
    while (w_rooks) {
        int sq = pop_lsb(w_rooks);
        int f = sq & 7;
        Bitboard file = FileMask[f];
        
        bool own_pawns = (w_pawns & file) != 0;
        bool opp_pawns = (b_pawns & file) != 0;
        
        if (!own_pawns && !opp_pawns) {
            mg_score += 25;
            eg_score += 20;
        } else if (!own_pawns) {
            mg_score += 15;
            eg_score += 12;
        }
    }
    
    Bitboard b_rooks = board.pieces[ROOK] & board.occupancy[BLACK];
    while (b_rooks) {
        int sq = pop_lsb(b_rooks);
        int f = sq & 7;
        Bitboard file = FileMask[f];
        
        bool own_pawns = (b_pawns & file) != 0;
        bool opp_pawns = (w_pawns & file) != 0;
        
        if (!own_pawns && !opp_pawns) {
            mg_score -= 25;
            eg_score -= 20;
        } else if (!own_pawns) {
            mg_score -= 15;
            eg_score -= 12;
        }
    }
}

static void eval_king_safety(const board_t& board, int& mg_score, int& eg_score) {
    Bitboard w_king = board.pieces[KING] & board.occupancy[WHITE];
    Bitboard b_king = board.pieces[KING] & board.occupancy[BLACK];
    
    if (!w_king || !b_king) return;
    
    int w_king_sq = std::countr_zero(w_king);
    int b_king_sq = std::countr_zero(b_king);
    
    Bitboard w_pawns = board.pieces[PAWN] & board.occupancy[WHITE];
    Bitboard b_pawns = board.pieces[PAWN] & board.occupancy[BLACK];
    
    int w_king_file = w_king_sq & 7;
    int w_king_rank = w_king_sq / 8;
    int w_shield = 0;
    
    if (w_king_rank <= 1) {
        for (int df = -1; df <= 1; df++) {
            int f = w_king_file + df;
            if (f < 0 || f > 7) continue;
            
            Bitboard file_mask = FileMask[f];
            Bitboard shield_zone = file_mask & (RankMask[1] | RankMask[2]);
            if (w_pawns & shield_zone) w_shield++;
        }
    }
    
    int b_king_file = b_king_sq & 7;
    int b_king_rank = b_king_sq / 8;
    int b_shield = 0;
    
    if (b_king_rank >= 6) {
        for (int df = -1; df <= 1; df++) {
            int f = b_king_file + df;
            if (f < 0 || f > 7) continue;
            
            Bitboard file_mask = FileMask[f];
            Bitboard shield_zone = file_mask & (RankMask[6] | RankMask[5]);
            if (b_pawns & shield_zone) b_shield++;
        }
    }
    
    mg_score += (w_shield - b_shield) * 12;
}

static void eval_castling(const board_t& board, int phase, int& mg_score, int& eg_score) {
    if (phase < 128) return;
    
    int castling_rights = board.history.back().castling_rights;
    
    Bitboard w_king = board.pieces[KING] & board.occupancy[WHITE];
    Bitboard b_king = board.pieces[KING] & board.occupancy[BLACK];
    
    if (!w_king || !b_king) return;
    
    int w_king_sq = std::countr_zero(w_king);
    int b_king_sq = std::countr_zero(b_king);
    
    bool w_castled_kingside = (w_king_sq == G1) && (board.pieces[ROOK] & board.occupancy[WHITE] & (1ULL << F1));
    bool w_castled_queenside = (w_king_sq == C1) && (board.pieces[ROOK] & board.occupancy[WHITE] & (1ULL << D1));
    bool w_has_castled = w_castled_kingside || w_castled_queenside;
    
    bool b_castled_kingside = (b_king_sq == G8) && (board.pieces[ROOK] & board.occupancy[BLACK] & (1ULL << F8));
    bool b_castled_queenside = (b_king_sq == C8) && (board.pieces[ROOK] & board.occupancy[BLACK] & (1ULL << D8));
    bool b_has_castled = b_castled_kingside || b_castled_queenside;
    
    bool w_can_castle_kingside = (castling_rights & (1 << 3)) != 0;
    bool w_can_castle_queenside = (castling_rights & (1 << 2)) != 0;
    bool b_can_castle_kingside = (castling_rights & (1 << 1)) != 0;
    bool b_can_castle_queenside = (castling_rights & (1 << 0)) != 0;
    
    bool w_can_castle = w_can_castle_kingside || w_can_castle_queenside;
    bool b_can_castle = b_can_castle_kingside || b_can_castle_queenside;
    
    constexpr int CASTLED_BONUS = 40;
    constexpr int LOST_RIGHTS_PENALTY = 25;
    constexpr int BOTH_RIGHTS_LOST_EXTRA = 15;
    
    int w_score = 0, b_score = 0;
    
    if (w_has_castled) {
        w_score += CASTLED_BONUS;
        if (w_castled_kingside) w_score += 5;
    } else {
        if (!w_can_castle_kingside) w_score -= LOST_RIGHTS_PENALTY;
        if (!w_can_castle_queenside) w_score -= LOST_RIGHTS_PENALTY - 5;
        if (!w_can_castle) w_score -= BOTH_RIGHTS_LOST_EXTRA;
    }
    
    if (b_has_castled) {
        b_score += CASTLED_BONUS;
        if (b_castled_kingside) b_score += 5;
    } else {
        if (!b_can_castle_kingside) b_score -= LOST_RIGHTS_PENALTY;
        if (!b_can_castle_queenside) b_score -= LOST_RIGHTS_PENALTY - 5;
        if (!b_can_castle) b_score -= BOTH_RIGHTS_LOST_EXTRA;
    }
    
    int scaled_diff = (w_score - b_score) * phase / 256;
    
    mg_score += scaled_diff;
    eg_score += (w_score - b_score) / 4;
}

static void eval_passed_pawns(const board_t& board, int& mg_score, int& eg_score) {
    Bitboard pawns = board.pieces[PAWN];
    
    static const int passed_bonus_mg[8] = {0, 5, 10, 20, 35, 60, 100, 0};
    static const int passed_bonus_eg[8] = {0, 10, 20, 40, 70, 120, 200, 0};
    
    while (pawns) {
        int sq = pop_lsb(pawns);
        int f = sq & 7;
        int r = sq / 8;
        
        Color Us = (board.occupancy[BLACK] >> sq) & 1ULL ? BLACK : WHITE;
        Bitboard opp_pawns = board.occupancy[~Us] & board.pieces[PAWN];
        
        Bitboard files_to_check = FileMask[f];
        if (f != 0) files_to_check |= FileMask[f - 1];
        if (f != 7) files_to_check |= FileMask[f + 1];
        
        Bitboard ranks_ahead;
        if (Us == WHITE) {
            ranks_ahead = ~((1ULL << ((r + 1) * 8)) - 1);
        } else {
            ranks_ahead = (1ULL << (r * 8)) - 1;
        }
        
        bool is_passed = !(opp_pawns & files_to_check & ranks_ahead);
        
        if (is_passed) {
            int advance = (Us == WHITE) ? r : (7 - r);
            int bonus_mg = passed_bonus_mg[advance];
            int bonus_eg = passed_bonus_eg[advance];
            
            if (Us == WHITE) {
                mg_score += bonus_mg;
                eg_score += bonus_eg;
            } else {
                mg_score -= bonus_mg;
                eg_score -= bonus_eg;
            }
        }
    }
}

static void eval_king_activity(const board_t& board, int& mg_score, int& eg_score) {
    Bitboard w_king = board.pieces[KING] & board.occupancy[WHITE];
    Bitboard b_king = board.pieces[KING] & board.occupancy[BLACK];
    
    if (!w_king || !b_king) return;
    
    int wk = std::countr_zero(w_king);
    int bk = std::countr_zero(b_king);
    
    auto center_dist = [](int sq) {
        int r = sq / 8, f = sq & 7;
        int dr = std::min(std::abs(r - 3), std::abs(r - 4));
        int df = std::min(std::abs(f - 3), std::abs(f - 4));
        return std::max(dr, df);
    };
    
    int w_center = center_dist(wk);
    int b_center = center_dist(bk);
    
    eg_score += (b_center - w_center) * 15;
}

static int edge_dist(int k) { 
    int r = k / 8, f = k & 7;
    return std::min({r, 7 - r, f, 7 - f});
}

static int corner_dist(int k) {
    int f = k & 7, r = k / 8;
    int d1 = std::max(f, r);
    int d2 = std::max(f, 7 - r);
    int d3 = std::max(7 - f, r);
    int d4 = std::max(7 - f, 7 - r);
    return std::min({d1, d2, d3, d4});
}

static bool check_forced_win_pos(const board_t& board) {
    if (board.pieces[PAWN]) return false;

    int nr_w = std::popcount(board.occupancy[WHITE]);
    int nr_b = std::popcount(board.occupancy[BLACK]);

    int queens_w = std::popcount(board.pieces[QUEEN] & board.occupancy[WHITE]);
    int queens_b = std::popcount(board.pieces[QUEEN] & board.occupancy[BLACK]);
    int rooks_w = std::popcount(board.pieces[ROOK] & board.occupancy[WHITE]);
    int rooks_b = std::popcount(board.pieces[ROOK] & board.occupancy[BLACK]);
    int bishops_w = std::popcount(board.pieces[BISHOP] & board.occupancy[WHITE]);
    int bishops_b = std::popcount(board.pieces[BISHOP] & board.occupancy[BLACK]);
    int knights_w = std::popcount(board.pieces[KNIGHT] & board.occupancy[WHITE]);
    int knights_b = std::popcount(board.pieces[KNIGHT] & board.occupancy[BLACK]);

    if (nr_b == 1 && (queens_w || rooks_w || bishops_w >= 2 || (knights_w && bishops_w)))
        return true;
    if (nr_w == 1 && (queens_b || rooks_b || bishops_b >= 2 || (knights_b && bishops_b)))
        return true;

    return false;
}

static void eval_king_confinement(const board_t& board, int& mg_score, int& eg_score) {
    if (!check_forced_win_pos(board)) return;

    Color loser = std::popcount(board.occupancy[WHITE]) == 1 ? WHITE : BLACK;
    int k = std::countr_zero(board.pieces[KING] & board.occupancy[loser]);
    
    int confinement = (3 - edge_dist(k)) * 2 + (7 - corner_dist(k));
    int bonus = confinement * 50;
    
    if (loser == WHITE) {
        eg_score -= bonus;
    } else {
        eg_score += bonus;
    }
}

static void eval_development(const board_t& board, int phase, int& mg_score) {
    if (phase < 200) return;
    
    int score = 0;
    
    Bitboard w_knights = board.pieces[KNIGHT] & board.occupancy[WHITE];
    if (w_knights & (1ULL << B1)) score -= 25;
    if (w_knights & (1ULL << G1)) score -= 25;
    
    Bitboard b_knights = board.pieces[KNIGHT] & board.occupancy[BLACK];
    if (b_knights & (1ULL << B8)) score += 25;
    if (b_knights & (1ULL << G8)) score += 25;
    
    Bitboard w_bishops = board.pieces[BISHOP] & board.occupancy[WHITE];
    if (w_bishops & (1ULL << C1)) score -= 20;
    if (w_bishops & (1ULL << F1)) score -= 20;
    
    Bitboard b_bishops = board.pieces[BISHOP] & board.occupancy[BLACK];
    if (b_bishops & (1ULL << C8)) score += 20;
    if (b_bishops & (1ULL << F8)) score += 20;
    
    Bitboard w_queen = board.pieces[QUEEN] & board.occupancy[WHITE];
    if (w_queen && !(w_queen & (1ULL << D1))) {
        score -= 15;
    }
    
    Bitboard b_queen = board.pieces[QUEEN] & board.occupancy[BLACK];
    if (b_queen && !(b_queen & (1ULL << D8))) {
        score += 15;
    }
    
    mg_score += score;
}

int evaluate_board(board_t& board) {
    int mg_score = 0, eg_score = 0;
    int phase = compute_phase(board);
    
    eval_material_pst(board, mg_score, eg_score);
    eval_mobility(board, mg_score, eg_score);
    eval_pawn_structure(board, mg_score, eg_score);
    eval_bishop_pair(board, mg_score, eg_score);
    eval_rook_files(board, mg_score, eg_score);
    eval_king_safety(board, mg_score, eg_score);
    eval_castling(board, phase, mg_score, eg_score);
    eval_passed_pawns(board, mg_score, eg_score);
    eval_king_activity(board, mg_score, eg_score);
    eval_king_confinement(board, mg_score, eg_score);
    eval_development(board, phase, mg_score);
    
    return taper(mg_score, eg_score, phase);
}

void print_evaluation_bar(board_t& board) {
    int cp = evaluate_board(board);
    double eval = cp / 100.0;
    double s = 50 + (eval / 10.0) * 50;
    s = std::clamp(s, 0.0, 100.0);
    int n = int(s / 5);

    std::cout << "[";
    for (int i = 0; i < n; i++) std::cout << "*";
    for (int i = n; i < 20; i++) std::cout << "-";
    std::cout << "] " << eval << "\n";
}
