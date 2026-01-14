#include "evaluationbar.h"
#include "board.h"
#include "move.h"
#include "bitboard.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

static int piece_value(int p) {
    switch (p) {
        case PAWN:   return 100;
        case KNIGHT: return 320;
        case BISHOP: return 330;
        case ROOK:   return 500;
        case QUEEN:  return 900;
        case KING:   return 20000;
        default:     return 0;
    }
}

static int mirror_sq(int sq) { return sq ^ 56; }
static const int pst[6][64] = {
};

static int eval_material_pst(board_t& board) {
    int score = 0;

    Bitboard pieces = board.occupancy[BOTH];

    while(pieces) {
        int sq = pop_lsb(pieces);
        int p = board.mailbox[sq];
        if (p == NONE) continue;

        bool black = (board.occupancy[BLACK] >> sq) & 1ULL;
        int val = piece_value(p);
        int psq = pst[p - 1][black ? mirror_sq(sq) : sq];
        score += black ? -(val + psq) : +(val + psq);
    }

    return score;
}

static int mobility_for (board_t& board, Color c){
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

static int eval_mobility(board_t& board) {
    int w = mobility_for(board, WHITE);
    int b = mobility_for(board, BLACK);
    return (w - b) * 4;
}

static int eval_pawn_structure(board_t& board) {
    int score = 0;
    int w[8] = {0}, b[8] = {0};
    
    for (int f = 0; f < 8; f++)
    {
        b[f] = std::popcount(board.occupancy[BLACK] & board.pieces[PAWN] & FileMask[f]);
        w[f] = std::popcount(board.occupancy[WHITE] & board.pieces[PAWN] & FileMask[f]);
    }

    for (int f = 0; f < 8; f++) {
        if (w[f] >= 2) score -= 20;
        if (b[f] >= 2) score += 20;

        if (w[f] == 1 && f > 0 && f < 7 && !w[f - 1] && !w[f + 1])
            score -= 15;
        if (b[f] == 1 && f > 0 && f < 7 && !b[f - 1] && !b[f + 1])
            score += 15;
    }
    return score;
}

static double phase(board_t &board) {
    int m = 0;

    m += std::popcount(board.pieces[PAWN]);
    m += 3 * std::popcount(board.pieces[KNIGHT]);
    m += 3 * std::popcount(board.pieces[BISHOP]);
    m += 5 * std::popcount(board.pieces[ROOK]);
    m += 9 * std::popcount(board.pieces[QUEEN]);

    return std::clamp(m / 40.0, 0.0, 1.0); // m max is 78
}

static int eval_opening_development(board_t &board) {
    if (phase(board) < 0.75) return 0;

    int score = 0;

    Bitboard occupancy = board.occupancy[BOTH];

    while (occupancy) {
        int sq = pop_lsb(occupancy);
        int p = board.mailbox[sq];
        bool black = (board.occupancy[BLACK] >> sq) & 1ULL;

        // Penalize early queen
        if (p == QUEEN) {
            int rank = sq / 8;
            if ((!black && rank > 1) || (black && rank < 6))
                score += black ? 40 : -40;
        }

        // Reward developed minors
        if (p == KNIGHT || p == BISHOP) {
            int r = sq / 8;
            if ((!black && r >= 2) || (black && r <= 5))
                score += black ? -35 : 35;
        }

        // Central pawns
        if (p == PAWN) {
            int f = sq & 7;
            if (f == 3 || f == 4)
                score += black ? -25 : 25;
        }
    }
    return score;
}

static int eval_king_activity(board_t &board) {
    if (phase(board) < 0.35) return 0; // penalize moving king to the center in early/mid game

    int wk = std::countr_zero(board.pieces[KING] & board.occupancy[WHITE]);
    int bk = std::countr_zero(board.pieces[KING] & board.occupancy[BLACK]);

    auto center_dist = [](int sq) {
        int r = sq / 8, f = sq & 7;
        return std::abs(r - 3) + std::abs(f - 3);
    };

    int score = (center_dist(bk) - center_dist(wk)) * 15;

    int dr = std::abs((wk / 8) - (bk / 8));
    int df = std::abs((wk & 7) - (bk & 7));
    if ((dr == 2 && df == 0) || (dr == 0 && df == 2))
        score += 40;

    return score;
}

static int eval_passed_pawns(board_t &board) {
    if (phase(board) > 0.5) return 0;

    int score = 0;

    Bitboard pawns = board.pieces[PAWN];
    int w[8] = {}, b[8] = {};

    for (int f = 0; f < 8; f++)
    {
        b[f] = std::popcount(board.occupancy[BLACK] & board.pieces[PAWN] & FileMask[f]);
        w[f] = std::popcount(board.occupancy[WHITE] & board.pieces[PAWN] & FileMask[f]);
    }

    while (pawns) 
    {
        int sq = pop_lsb(pawns);
        int f = sq & 7;
        int r = sq / 8;

        Color Us = (board.occupancy[BLACK] >> sq) & 1ULL ? BLACK : WHITE;
        int Up = Us == WHITE ? 8 : -8;

        Bitboard opp_pawns = board.occupancy[~Us] & board.pieces[PAWN];
        Bitboard files_to_check = FileMask[f];
        if (f != 0) files_to_check |= FileMask[f - 1];
        if (f != 7) files_to_check |= FileMask[f + 1];

        Bitboard ranks_to_check = Us == WHITE ? ~(((1ULL << (r * 8)) - 1) | RankMask[r]) : ((1ULL << (r * 8)) - 1);

        bool pawns_in_front = opp_pawns & files_to_check & ranks_to_check;
        if (!pawns_in_front)
            continue;

        int advance = Us == BLACK ? (7 - r) : r;
        score += Us == BLACK ? -(advance * 20) : (advance * 20);
    }

    return score;
}

static int edge_dist(int k){ 
    int r = k / 8, f = k & 7;
    return std::min({r, 7 - r, f, 7 - f}); // distance to edge
}

static int corner_dist(int k) {
    int f = k & 7, r = k / 8;
    
    int d1 = std::max(f, r);
    int d2 = std::max(f, 7 - r);
    int d3 = std::max(7 - f, r);
    int d4 = std::max(7 - f, 7 - r);
    
    return std::min({ d1, d2, d3, d4 });
}

// check for positions of the type KRK, KBBK, KQK, KBNK
static bool check_forced_win_pos(board_t& board) {
    if (board.pieces[PAWN])
        return false;

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

    if (nr_b == 1 && (queens_w || rooks_w|| bishops_w >= 2 || (knights_w && bishops_w))) // WHITE winning
        return true;

    if (nr_w == 1 && (queens_b || rooks_b || bishops_b >= 2 || (knights_b && bishops_b))) // BALCK winning
        return true;

    return false;
}

static int eval_king_confinement(board_t &board) {
    if (!check_forced_win_pos(board)) return 0; // only for KRK, KBBK, KQK

    Color loser = std::popcount(board.occupancy[WHITE]) == 1 ? WHITE : BLACK;

    int k = std::countr_zero(board.pieces[KING] & board.occupancy[loser]);
    
    int confinement = (3 - edge_dist(k)) * 2 + (7 - corner_dist(k));
    int score = confinement * 40 * (loser == WHITE ? -1 : 1);

    return score;
}

int evaluate_board(board_t& board) {
    int score = 0;

    score += int(1.1 * eval_material_pst(board));
    score += eval_mobility(board);
    score += eval_pawn_structure(board);

    score += eval_opening_development(board);

    score += eval_king_activity(board);
    score += eval_passed_pawns(board);
    score += eval_king_confinement(board);

    return score;
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


//use these to run in MSY2 :
// cd "/c/Users/THINKPAD E16 I7/Documents/sheriff-ai"
//rm -rf build
//ls
// cmake -S . -B build \
//   -G "Unix Makefiles" \
//   -DCMAKE_BUILD_TYPE=Release \
//   -DCMAKE_CXX_FLAGS="-O2"
//cmake --build build -j
// ./build/sheriff_ai_lichess.exe
