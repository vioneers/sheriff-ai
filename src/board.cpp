#include "board.h"
#include "bitboard.h"

board_t::board_t()
{
	Bitboard rank1 = (1ULL<<8) - 1;

	pieces[PAWN]   = (rank1 << 8) | (rank1 << (8*6)); 
    pieces[KNIGHT] = (1ULL<<B1) | (1ULL<<G1) | (1ULL<<B8) | (1ULL<<G8);
    pieces[BISHOP] = (1ULL<<C1) | (1ULL<<F1) | (1ULL<<C8) | (1ULL<<F8);
    pieces[ROOK]   = (1ULL<<A1) | (1ULL<<H1) | (1ULL<<A8) | (1ULL<<H8); 
    pieces[QUEEN]  = (1ULL<<D1) | (1ULL<<D8);
    pieces[KING]   = (1ULL<<E1) | (1ULL<<E8);

	occupancy[WHITE] = rank1 | (rank1 << 8); // ranks 1 and 2
	occupancy[BLACK] = (rank1 << (8*7)) | (rank1 << (8*6)); // ranks 7 and 8
	occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];

	// Initialize the Mailbox
	for (int i = 0; i < 64; i++) {
        mailbox[i] = NONE; 
    }

    // Iterate over piece types (PAWN to KING)
    for (int p = PAWN; p <= KING; p++) {
        uint64_t bb = pieces[p];

        while (bb) {
            // Get the index of the Least Significant Bit (LSB)
            int sq = __builtin_ctzll(bb); 

            // Determine color by checking the White occupancy board
            int color = (occupancy[WHITE] & (1ULL << sq)) ? WHITE : BLACK;

            mailbox[sq] = (color == WHITE) ? p : -p; 

            // Remove LSB
            bb &= (bb - 1); 
        }
    }

	state_t init{0, (1<<4)-1, -1, NONE, WHITE}; // z_key, castling_rights, ep_square, captured, turn
	history.push_back{init};
}

board_t::board_t(std::string fen)
{
	//TODO: init board from fen format
}

board_t::board_t(std::vector <move_t> move_hist): board_t()
{
	for(auto& move : move_hist)
		make_move(move);
}

// Making and unmaking moves

bool board_t::make_move(move_t m){
	if (history.empty())
		return false;

	const state_t& prev = history.back();
	Color us = prev.turn;
	Color them = ~us;

	int from = m.from();
	int to = m.to();
	Bitboard from_bb = 1ULL << from;
	Bitboard to_bb = 1ULL << to;

	int signed_piece = mailbox[from];
	if (signed_piece == NONE)
		return false;

	PieceType moving = (signed_piece < 0) ? (PieceType)(-signed_piece) : (PieceType)signed_piece;
	int flag = m.flag();

	bool is_ep = (flag == EP_CAPTURE);
	bool is_castle = (flag == K_CASTLE || flag == Q_CASTLE);
	bool is_promo = (flag & 0b1000) != 0;
	bool is_double = (flag == DOUBLE_PUSH);
	bool is_capture = is_ep || (flag & CAPTURE) || (occupancy[them] & to_bb);

	state_t next = prev;
	next.turn = them;
	next.ep_square = -1;
	next.captured = NONE;

	if (is_capture && !is_ep) {
		int cap_signed = mailbox[to];
		PieceType captured = (cap_signed < 0) ? (PieceType)(-cap_signed) : (PieceType)cap_signed;
		if (captured == NONE) {
			for (int p = PAWN; p <= KING; ++p) {
				if (pieces[p] & to_bb) {
					captured = (PieceType)p;
					break;
				}
			}
		}
		if (captured != NONE) {
			pieces[captured] &= ~to_bb;
			occupancy[them] &= ~to_bb;
			next.captured = captured;
		}
	}

	if (is_ep) {
		int cap_sq = to + (us == WHITE ? -8 : 8);
		Bitboard cap_bb = 1ULL << cap_sq;
		pieces[PAWN] &= ~cap_bb;
		occupancy[them] &= ~cap_bb;
		mailbox[cap_sq] = NONE;
		next.captured = PAWN;
	}

	pieces[moving] &= ~from_bb;
	occupancy[us] &= ~from_bb;
	mailbox[from] = NONE;

	if (is_promo) {
		PieceType promo = QUEEN;
		switch (flag) {
			case PROMO_N:
			case PROMO_N_CAP:
				promo = KNIGHT;
				break;
			case PROMO_B:
			case PROMO_B_CAP:
				promo = BISHOP;
				break;
			case PROMO_R:
			case PROMO_R_CAP:
				promo = ROOK;
				break;
			default:
				promo = QUEEN;
				break;
		}

		pieces[promo] |= to_bb;
		occupancy[us] |= to_bb;
		mailbox[to] = (us == WHITE) ? promo : (PieceType)(-promo);
	} else {
		pieces[moving] |= to_bb;
		occupancy[us] |= to_bb;
		mailbox[to] = (us == WHITE) ? moving : (PieceType)(-moving);
	}

	if (is_castle && moving == KING) {
		int rook_from = -1;
		int rook_to = -1;
		if (us == WHITE) {
			if (flag == K_CASTLE) { rook_from = H1; rook_to = F1; }
			else { rook_from = A1; rook_to = D1; }
		} else {
			if (flag == K_CASTLE) { rook_from = H8; rook_to = F8; }
			else { rook_from = A8; rook_to = D8; }
		}
		Bitboard rook_from_bb = 1ULL << rook_from;
		Bitboard rook_to_bb = 1ULL << rook_to;
		pieces[ROOK] &= ~rook_from_bb;
		pieces[ROOK] |= rook_to_bb;
		occupancy[us] &= ~rook_from_bb;
		occupancy[us] |= rook_to_bb;
		mailbox[rook_from] = NONE;
		mailbox[rook_to] = (us == WHITE) ? ROOK : (PieceType)(-ROOK);
	}

	int rights = prev.castling_rights;
	if (moving == KING) {
		if (us == WHITE)
			rights &= ~((1 << 0) | (1 << 1));
		else
			rights &= ~((1 << 2) | (1 << 3));
	}
	if (moving == ROOK) {
		if (us == WHITE) {
			if (from == H1) rights &= ~(1 << 0);
			else if (from == A1) rights &= ~(1 << 1);
		} else {
			if (from == H8) rights &= ~(1 << 2);
			else if (from == A8) rights &= ~(1 << 3);
		}
	}
	if (is_capture && !is_ep) {
		if (to == H1) rights &= ~(1 << 0);
		else if (to == A1) rights &= ~(1 << 1);
		else if (to == H8) rights &= ~(1 << 2);
		else if (to == A8) rights &= ~(1 << 3);
	}

	next.castling_rights = rights;
	if (is_double)
		next.ep_square = from + (us == WHITE ? 8 : -8);

	occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];
	history.push_back(next);
	return true;
}

void board_t::undo_move(move_t m)
{
	if (history.size() < 2)
		return;

	const state_t last = history.back();
	history.pop_back();

	Color them = last.turn;
	Color us = ~them;

	int from = m.from();
	int to = m.to();
	Bitboard from_bb = 1ULL << from;
	Bitboard to_bb = 1ULL << to;

	int flag = m.flag();
	bool is_ep = (flag == EP_CAPTURE);
	bool is_castle = (flag == K_CASTLE || flag == Q_CASTLE);
	bool is_promo = (flag & 0b1000) != 0;

	PieceType moving = PAWN;

	if (is_promo) {
		PieceType promo = QUEEN;
		switch (flag) {
			case PROMO_N:
			case PROMO_N_CAP:
				promo = KNIGHT;
				break;
			case PROMO_B:
			case PROMO_B_CAP:
				promo = BISHOP;
				break;
			case PROMO_R:
			case PROMO_R_CAP:
				promo = ROOK;
				break;
			default:
				promo = QUEEN;
				break;
		}

		pieces[promo] &= ~to_bb;
		occupancy[us] &= ~to_bb;
		mailbox[to] = NONE;
	} else {
		int signed_piece = mailbox[to];
		moving = (signed_piece < 0) ? (PieceType)(-signed_piece) : (PieceType)signed_piece;
		pieces[moving] &= ~to_bb;
		occupancy[us] &= ~to_bb;
		mailbox[to] = NONE;
	}

	pieces[moving] |= from_bb;
	occupancy[us] |= from_bb;
	mailbox[from] = (us == WHITE) ? moving : (PieceType)(-moving);

	if (is_castle && moving == KING) {
		int rook_from = -1;
		int rook_to = -1;
		if (us == WHITE) {
			if (flag == K_CASTLE) { rook_from = H1; rook_to = F1; }
			else { rook_from = A1; rook_to = D1; }
		} else {
			if (flag == K_CASTLE) { rook_from = H8; rook_to = F8; }
			else { rook_from = A8; rook_to = D8; }
		}
		Bitboard rook_from_bb = 1ULL << rook_from;
		Bitboard rook_to_bb = 1ULL << rook_to;
		pieces[ROOK] &= ~rook_to_bb;
		pieces[ROOK] |= rook_from_bb;
		occupancy[us] &= ~rook_to_bb;
		occupancy[us] |= rook_from_bb;
		mailbox[rook_to] = NONE;
		mailbox[rook_from] = (us == WHITE) ? ROOK : (PieceType)(-ROOK);
	}

	if (last.captured != NONE) {
		if (is_ep) {
			int cap_sq = to + (us == WHITE ? -8 : 8);
			Bitboard cap_bb = 1ULL << cap_sq;
			pieces[PAWN] |= cap_bb;
			occupancy[them] |= cap_bb;
			mailbox[cap_sq] = (them == WHITE) ? PAWN : (PieceType)(-PAWN);
		} else {
			pieces[last.captured] |= to_bb;
			occupancy[them] |= to_bb;
			mailbox[to] = (them == WHITE) ? (PieceType)last.captured : (PieceType)(-last.captured);
		}
	}

	occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];
}

// Other utilities
std::string to_fen()
{
	//TODO: convert board to fen string for debugging
}

// Check if a square is attacked by any piece of the given color using bitboard rays/lookup tables
bool board_t::square_attacked(int sq, Color by_color) const
{
	Bitboard target = 1ULL << sq;
	Bitboard occ_all = occupancy[BOTH];
	Bitboard by_occ = occupancy[by_color];

	// Pawn attacks (flip directions per color)
	Bitboard pawns = pieces[PAWN] & by_occ;
	if (by_color == WHITE) {
		Bitboard attacks = shift<7>(pawns & ~FileMask[0]) | shift<9>(pawns & ~FileMask[7]);
		if (attacks & target) return true;
	} else if (by_color == BLACK) {
		Bitboard attacks = shift<-7>(pawns & ~FileMask[7]) | shift<-9>(pawns & ~FileMask[0]);
		if (attacks & target) return true;
	}

	// Knight attacks (symmetric)
	Bitboard knights = pieces[KNIGHT] & by_occ;
	if (KnightAttacks[sq] & knights)
		return true;

	// King attacks (symmetric)
	Bitboard kings = pieces[KING] & by_occ;
	if (KingAttacks[sq] & kings)
		return true;

	// Sliding attacks
	auto ray_hit = [&](int df, int dr, Bitboard sliders) -> bool {
		int f = (sq % 8) + df;
		int r = (sq / 8) + dr;
		while (f >= 0 && f < 8 && r >= 0 && r < 8) {
			int idx = r * 8 + f;
			Bitboard bb = 1ULL << idx;
			if (occ_all & bb)
				return (sliders & bb) != 0;
			f += df;
			r += dr;
		}
		return false;
	};

	// Bishop and queen diagonals
	Bitboard bishops = (pieces[BISHOP] | pieces[QUEEN]) & by_occ;
	if (ray_hit(1, 1, bishops) || ray_hit(-1, 1, bishops) ||
		ray_hit(1, -1, bishops) || ray_hit(-1, -1, bishops))
		return true;

	// Rook and queen orthogonals
	Bitboard rooks = (pieces[ROOK] | pieces[QUEEN]) & by_occ;
	if (ray_hit(0, 1, rooks) || ray_hit(0, -1, rooks) ||
		ray_hit(1, 0, rooks) || ray_hit(-1, 0, rooks))
		return true;

	return false;
}

// Is the given color currently in check?
bool board_t::in_check(Color color) const
{
	Bitboard king_bb = pieces[KING] & occupancy[color];
	if (!king_bb)
		return false; // king missing; treat as not in check

	int king_sq = __builtin_ctzll(king_bb);
	return square_attacked(king_sq, ~color);
}

void board_t::add_move(std::vector<move_t> &list, move_t move)
{
	Color us = history.empty() ? WHITE : history.back().turn;

	if (!make_move(move))
		return;

	if (!in_check(us))
		list.push_back(move);

	undo_move(move);
}

void board_t::get_legal_moves(std::vector<move_t> &list, Color color)
{
	list.clear();

	std::vector<move_t> pseudo;
	pseudo.reserve(128);

	if (color == WHITE) {
		generate_pawn_moves<WHITE>(pseudo);
		generate_knight_moves<WHITE>(pseudo);
		generate_bishop_moves<WHITE>(pseudo);
		generate_rook_moves<WHITE>(pseudo);
		generate_queen_moves<WHITE>(pseudo);
		generate_king_moves<WHITE>(pseudo);
	} else {
		generate_pawn_moves<BLACK>(pseudo);
		generate_knight_moves<BLACK>(pseudo);
		generate_bishop_moves<BLACK>(pseudo);
		generate_rook_moves<BLACK>(pseudo);
		generate_queen_moves<BLACK>(pseudo);
		generate_king_moves<BLACK>(pseudo);
	}

	for (const auto& move : pseudo)
		add_move(list, move);
}
