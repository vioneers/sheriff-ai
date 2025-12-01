#include "engine.h"
#include <vector>

engine_t::engine_t(std::vector<move_t> move_hist)
    : pawn_w(false),
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
