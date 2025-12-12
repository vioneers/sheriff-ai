#include "board.h"
#include "bitboard.h"

template<Color Us> 
void board_t::generate_pawn_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	constexpr int Up     = (Us == WHITE) ?  8 : -8;
    constexpr int Right  = (Us == WHITE) ?  9 : -7;
    constexpr int Left   = (Us == WHITE) ?  7 : -9;

	constexpr Bitboard FileA = 0x0101010101010101ULL;
	constexpr Bitboard FileH = 0x8080808080808080ULL;
	constexpr Bitboard StartRank = (Us == WHITE) ? 0x000000000000FF00ULL : 0x00FF000000000000ULL;
	constexpr Bitboard PromoRank = (Us == WHITE) ? 0xFF00000000000000ULL : 0x00000000000000FFULL;

	Bitboard pawns = pieces[PAWN] & occupancy[Us];
	Bitboard empty = ~occupancy[BOTH];

	auto add_promos = [&](int from, int to, bool capture) {
		if (capture) {
			list.emplace_back(from, to, PROMO_N_CAP);
			list.emplace_back(from, to, PROMO_B_CAP);
			list.emplace_back(from, to, PROMO_R_CAP);
			list.emplace_back(from, to, PROMO_Q_CAP);
		} else {
			list.emplace_back(from, to, PROMO_N);
			list.emplace_back(from, to, PROMO_B);
			list.emplace_back(from, to, PROMO_R);
			list.emplace_back(from, to, PROMO_Q);
		}
	};

	// Single pushes
	Bitboard single = shift<Up>(pawns) & empty;
	Bitboard single_promos = single & PromoRank;
	Bitboard single_quiet  = single & ~PromoRank;

	while (single_quiet) {
		int to = __builtin_ctzll(single_quiet);
		int from = to - Up;
		list.emplace_back(from, to, QUIET);
		single_quiet &= single_quiet - 1;
	}
	while (single_promos) {
		int to = __builtin_ctzll(single_promos);
		int from = to - Up;
		add_promos(from, to, false);
		single_promos &= single_promos - 1;
	}

	// Double pushes from starting rank
	Bitboard start_pawns = pawns & StartRank;
	Bitboard single_from_start = shift<Up>(start_pawns) & empty;
	Bitboard double_push = shift<Up>(single_from_start) & empty;
	while (double_push) {
		int to = __builtin_ctzll(double_push);
		int from = to - 2 * Up;
		list.emplace_back(from, to, DOUBLE_PUSH);
		double_push &= double_push - 1;
	}

	// Captures
	Bitboard right_caps = shift<Right>(pawns & ~FileH) & occupancy[Them];
	Bitboard left_caps  = shift<Left>(pawns & ~FileA) & occupancy[Them];

	Bitboard right_promos = right_caps & PromoRank;
	Bitboard left_promos  = left_caps & PromoRank;

	right_caps &= ~PromoRank;
	left_caps  &= ~PromoRank;

	while (right_caps) {
		int to = __builtin_ctzll(right_caps);
		int from = to - Right;
		list.emplace_back(from, to, CAPTURE);
		right_caps &= right_caps - 1;
	}
	while (left_caps) {
		int to = __builtin_ctzll(left_caps);
		int from = to - Left;
		list.emplace_back(from, to, CAPTURE);
		left_caps &= left_caps - 1;
	}
	while (right_promos) {
		int to = __builtin_ctzll(right_promos);
		int from = to - Right;
		add_promos(from, to, true);
		right_promos &= right_promos - 1;
	}
	while (left_promos) {
		int to = __builtin_ctzll(left_promos);
		int from = to - Left;
		add_promos(from, to, true);
		left_promos &= left_promos - 1;
	}

	// En passant
	int ep = history.empty() ? -1 : history.back().ep_square;
	if (ep != -1) {
		Bitboard ep_bb = 1ULL << ep;
		Bitboard right_ep = shift<Right>(pawns & ~FileH);
		Bitboard left_ep  = shift<Left>(pawns & ~FileA);

		if (ep_bb & right_ep)
			list.emplace_back(ep - Right, ep, EP_CAPTURE);
		if (ep_bb & left_ep)
			list.emplace_back(ep - Left, ep, EP_CAPTURE);
	}
}

template<Color Us> 
void generate_knight_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_king_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_queen_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_rook_moves(MoveList& list)
{
	// TODO:
}

template<Color Us> 
void generate_bishop_moves(MoveList& list)
{
	// TODO:
}
