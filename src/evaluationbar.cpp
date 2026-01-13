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

static int eval_material_pst(board_t* board) {
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        int p = board->mailbox[sq];
        if (p == NONE) continue;

        bool black = (board->occupancy[BLACK] >> sq) & 1ULL;
        int val = piece_value(p);
        int psq = pst[p - 1][black ? mirror_sq(sq) : sq];
        score += black ? -(val + psq) : +(val + psq);
    }
    return score;
}
static int mobility_for (board_t* board, Color c){
    Color turn = board->history.back().turn;
    Bitboard own = board->occupancy[turn];
    Bitboard occ = board->occupancy[BOTH];
    int count = 0;

    Bitboard knights = board->pieces[KNIGHT] & own;
    while (knights) {
        int sq = pop_lsb(knights);
        count += std::popcount(KnightAttacks[sq] & ~own);
    }

    Bitboard bishops = board->pieces[BISHOP] & own;
    while (bishops) {
        int sq = pop_lsb(bishops);
        Bitboard blockers = occ & BishopMask[sq];
        int hash = apply_magic(blockers, BMagic[sq], BShift[sq]);
        Bitboard attacks = BishopAttacks[sq][hash];
        count += std::popcount(attacks & ~own);
    }

    Bitboard rooks = board->pieces[ROOK] & own;
    while (rooks) {
        int sq = pop_lsb(rooks);
        Bitboard blockers = occ & RookMask[sq];
        int hash = apply_magic(blockers, RMagic[sq], RShift[sq]);
        Bitboard attacks = RookAttacks[sq][hash];
        count += std::popcount(attacks & ~own);
    }

    Bitboard queens = board->pieces[QUEEN] & own;
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
static int eval_mobility(board_t* board) {
    int w = mobility_for(board, WHITE);
    int b = mobility_for(board, BLACK);
    return (w - b) * 4;
}

static int eval_pawn_structure(board_t* board) {
    int score = 0;
    int w[8] = {0}, b[8] = {0};

    for (int sq = 0; sq < 64; sq++) {
        if (board->mailbox[sq] != PAWN) continue;
        bool black = (board->occupancy[BLACK] >> sq) & 1ULL;
        (black ? b : w)[sq & 7]++;
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

static double phase(board_t* board) {
    int m = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->mailbox[i]) {
            case PAWN: m += 1; break;
            case KNIGHT:
            case BISHOP: m += 3; break;
            case ROOK: m += 5; break;
            case QUEEN: m += 9; break;
            default: break;
        }
    }
    return std::clamp(m / 40.0, 0.0, 1.0);
}
static int eval_opening_development(board_t* board) {
    if (phase(board) < 0.75) return 0;

    int score = 0;

    for (int sq = 0; sq < 64; sq++) {
        int p = board->mailbox[sq];
        bool black = (board->occupancy[BLACK] >> sq) & 1ULL;

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
static int eval_king_activity(board_t* board) {
    if (phase(board) > 0.35) return 0;

    int wk = -1, bk = -1;
    for (int i = 0; i < 64; i++) {
        if (board->mailbox[i] == KING) {
            if ((board->occupancy[WHITE] >> i) & 1ULL) wk = i;
            else bk = i;
        }
    }
    if (wk == -1 || bk == -1) return 0;

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

static int eval_passed_pawns(board_t* board) {
    if (phase(board) > 0.5) return 0;

    int score = 0;

    for (int sq = 0; sq < 64; sq++) {
        if (board->mailbox[sq] != PAWN) continue;

        bool black = (board->occupancy[BLACK] >> sq) & 1ULL;
        int rank = sq / 8;

        bool passed = true;
        int f1 = sq & 7;
        int r1 = sq / 8;
        for (int i = 0; i < 64; i++) {
            if (board->mailbox[i] != PAWN) continue;
            bool opp = ((board->occupancy[black ? WHITE : BLACK] >> i) & 1ULL);
            if (!opp) continue;

            int f2 = i & 7;
            int r2 = i / 8; 
            if (std::abs(f1 - f2) > 1) continue;
            bool pawn_in_front = black ? (r2 < r1) : (r2 > r1); 
            if(!pawn_in_front)
                continue;
            passed = false; 
            break;
        }

        if (!passed) continue;

        int advance = black ? (7 - rank) : rank;
        score += black ? -(advance * 20) : (advance * 20);
    }

    return score;
}

static int confinement(int k){ 
    int r = k / 8, f = k & 7;
    int edge = std::min({r, 7 - r, f, 7 - f}); // distance to edge

    return (3 - edge) * 40;
}

static int eval_king_confinement(board_t* board) {
    if (phase(board) > 0.4) return 0;

    int wk = -1, bk = -1;
    for (int i = 0; i < 64; i++) {
        if (board->mailbox[i] == KING){ 
            if ((board->occupancy[BLACK] >> i) & 1ULL)
                bk = i; // black king
            else 
                wk = i; // white king
        }
    }
    if (wk == -1 || bk == -1) return 0;

    return confinement(bk) - confinement(wk);
}


int evaluate_board(board_t* board) {
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

void print_evaluation_bar(board_t* board) {
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
