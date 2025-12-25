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
		int to = pop_lsb(single_quiet);
		int from = to - Up;
		list.emplace_back(from, to, QUIET);
	}
	while (single_promos) {
		int to = pop_lsb(single_promos);
		int from = to - Up;
		add_promos(from, to, false);
	}

	// Double pushes from starting rank
	Bitboard start_pawns = pawns & StartRank;
	Bitboard single_from_start = shift<Up>(start_pawns) & empty;
	Bitboard double_push = shift<Up>(single_from_start) & empty;
	while (double_push) {
		int to = pop_lsb(double_push);
		int from = to - 2 * Up;
		list.emplace_back(from, to, DOUBLE_PUSH);
	}

	// Captures
	Bitboard right_caps = shift<Right>(pawns & ~FileH) & occupancy[Them];
	Bitboard left_caps  = shift<Left>(pawns & ~FileA) & occupancy[Them];

	Bitboard right_promos = right_caps & PromoRank;
	Bitboard left_promos  = left_caps & PromoRank;

	right_caps &= ~PromoRank;
	left_caps  &= ~PromoRank;

	while (right_caps) {
		int to = pop_lsb(right_caps);
		int from = to - Right;
		list.emplace_back(from, to, CAPTURE);
	}
	while (left_caps) {
		int to = pop_lsb(left_caps);
		int from = to - Left;
		list.emplace_back(from, to, CAPTURE);
	}
	while (right_promos) {
		int to = pop_lsb(right_promos);
		int from = to - Right;
		add_promos(from, to, true);
	}
	while (left_promos) {
		int to = pop_lsb(left_promos);
		int from = to - Left;
		add_promos(from, to, true);
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
void board_t::generate_knight_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	Bitboard knights = pieces[KNIGHT] & occupancy[Us];
	Bitboard own = occupancy[Us];
	Bitboard their = occupancy[Them];

	while (knights) {
		int from = pop_lsb(knights);
		Bitboard moves = KnightAttacks[from] & ~own;
		while (moves) {
			int to = pop_lsb(moves);

			bool is_capture = (their >> to) & 1ULL;
			list.emplace_back(from, to, is_capture ? CAPTURE : QUIET);
		}
	}
}

template<Color Us> 
void board_t::generate_king_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	Bitboard kings = pieces[KING] & occupancy[Us];
	Bitboard own = occupancy[Us];
	Bitboard their = occupancy[Them];

	while (kings) {
		int from = pop_lsb(kings);
		Bitboard moves = KingAttacks[from] & ~own;
		while (moves) {
			int to = pop_lsb(moves);

			bool is_capture = (their >> to) & 1ULL;
			list.emplace_back(from, to, is_capture ? CAPTURE : QUIET);
		}
	}
}

template<Color Us> 
void board_t::generate_queen_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	Bitboard queens = pieces[QUEEN] & occupancy[Us];
	Bitboard own = occupancy[Us];
	Bitboard their = occupancy[Them];

	while (queens) 
	{
		int from = pop_lsb(queens);

		Bitboard rblockers = occupancy[BOTH] & RookMask[from];
		Bitboard rhash = apply_magic(rblockers, RMagic[from], RShift[from]);

		Bitboard bblockers = occupancy[BOTH] & BishopMask[from];
		Bitboard bhash = apply_magic(bblockers, BMagic[from], BShift[from]);
		
		Bitboard moves = RookAttacks[from][rhash] | BishopAttacks[from][bhash];

		while (moves)
		{
			int to = pop_lsb(moves);
			bool is_capture = (their >> to) & 1ULL;
			list.emplace_back(from, to, is_capture ? CAPTURE : QUIET);
		}
	}
}

template<Color Us> 
void board_t::generate_rook_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	Bitboard rooks = pieces[ROOK] & occupancy[Us];
	Bitboard own = occupancy[Us];
	Bitboard their = occupancy[Them];

	while (rooks)
	{
		int from = pop_lsb(rooks);

		Bitboard blockers = occupancy[BOTH] & RookMask[from];
		Bitboard hash = apply_magic(blockers, RMagic[from], RShift[from]);

		Bitboard moves = RookAttacks[from][hash];

		while (moves)
		{
			int to = pop_lsb(moves);
			bool is_capture = (their >> to) & 1ULL;
			list.emplace_back(from, to, is_capture ? CAPTURE : QUIET);
		}
	}
}

template<Color Us> 
void board_t::generate_bishop_moves(std::vector<move_t> &list)
{
	constexpr Color Them = ~Us;
	Bitboard bishops = pieces[BISHOP] & occupancy[Us];
	Bitboard own = occupancy[Us];
	Bitboard their = occupancy[Them];

	while (bishops)
	{
		int from = pop_lsb(bishops);

		Bitboard blockers = occupancy[BOTH] & BishopMask[from];
		Bitboard hash = apply_magic(blockers, BMagic[from], BShift[from]);

		Bitboard moves = BishopAttacks[from][hash];

		while (moves)
		{
			int to = pop_lsb(moves);
			bool is_capture = (their >> to) & 1ULL;
			list.emplace_back(from, to, is_capture ? CAPTURE : QUIET);
		}
	}
}