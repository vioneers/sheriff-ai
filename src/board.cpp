#include <bit>
#include <sstream>

#include "board.h"
#include "bitboard.h"
#include "debug.h"

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

    // Iterate over piece types
    for (PieceType p : AllPieceTypes) {
        uint64_t bb = pieces[p];

        while (bb) {
            // Get the index of the Least Significant Bit (LSB) and pop it
            int sq = pop_lsb(bb); 

            // Determine color by checking the White occupancy board
            int color = (occupancy[WHITE] & (1ULL << sq)) ? WHITE : BLACK;

            mailbox[sq] = p; 
        }
    }

	history.push_back({0, (1 << 4) - 1, -1, NONE, WHITE}); // z_key, castling_rights, ep_square, captured, turn
}

board_t::board_t(std::string fen)
{
	// 0. Reset state
	for (auto p : AllPieceTypes) pieces[p] = 0;
	occupancy[WHITE] = occupancy[BLACK] = occupancy[BOTH] = 0;
	for (int i = 0; i < 64; ++i) mailbox[i] = NONE;

	std::stringstream ss(fen);
	std::string pos, side, castling, ep;
	ss >> pos >> side >> castling >> ep;

	// 1. Pieces
	int rank = 7, file = 0;
	for (char c : pos) {
		if (c == '/') { rank--; file = 0; }
		else if (isdigit(c)) { file += (c - '0'); }
		else {
			int sq = rank * 8 + file;
			
			Color col = isupper(c) ? WHITE : BLACK;
			char lower_c = tolower(c);
			PieceType type;

			switch (lower_c) {
			case 'p': type = PAWN;   break;
			case 'n': type = KNIGHT; break;
			case 'b': type = BISHOP; break;
			case 'r': type = ROOK;   break;
			case 'q': type = QUEEN;  break;
			case 'k': type = KING;   break;
			default:  type = NONE;   break;
			}

			if (type != NONE)
			{
				pieces[type] |= (1ULL << sq);
				occupancy[col] |= (1ULL << sq);
				mailbox[sq] = type;
				// std::cout << lower_c << " " << sq << '\n';
			}
			
			file++;
		}
	}
	occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];

	// 2. Turn
	state_t current;
	current.turn = (side == "w") ? WHITE : BLACK;

	// 3. Castling
	current.castling_rights = 0;
	if (castling.find('K') != std::string::npos) current.castling_rights |= (1 << 3);
	if (castling.find('Q') != std::string::npos) current.castling_rights |= (1 << 2);
	if (castling.find('k') != std::string::npos) current.castling_rights |= (1 << 1);
	if (castling.find('q') != std::string::npos) current.castling_rights |= (1 << 0);

	// 4. EP
	auto string_to_sq = [&](std::string code) {
		int rank = code[1] - '1';
		int file = code[0] - 'a';
		return rank * 8 + file;
	};
	current.ep_square = (ep == "-") ? -1 : string_to_sq(ep);

	history.push_back(current);
}

board_t::board_t(std::vector <move_t> move_hist): board_t()
{
	for(auto& move : move_hist)
		make_move(move, true);
}

// Making and unmaking moves
// Assumes moves are pseudolegal
bool board_t::make_move(move_t &m, bool apply_flags){
	if (history.empty())
		return false;

	const state_t& prev = history.back();
	Color Us = prev.turn;
	Color Them = ~Us;
	int Up = (Us == WHITE) ? 8 : -8;

	int from = m.from();
	int to = m.to();
	Bitboard from_bb = 1ULL << from;
	Bitboard to_bb = 1ULL << to;

	PieceType moving = mailbox[from];
	if (moving == NONE)
		return false;

	int flag = m.flag();

	// Apply correct flags to the move object
	#ifndef DEBUG // If in debug mode, always recompute the flags
	if (apply_flags) 
	#endif
	{
		if (moving == PAWN) {

			Bitboard PromoRank = (Us == WHITE) ? RankMask[7] : RankMask[0];

			if (flag == QUIET) // Skip if flag is already set (ie for promotion)
				// Only check for double push and en passant, as promotion is handled when converting from UCI format
				if (to == from + 2 * Up)
					flag = DOUBLE_PUSH;
				else if (prev.ep_square != -1 && to == prev.ep_square) // If moving pawn to ep_square => en passant capture
					flag = EP_CAPTURE;
					// Normal pawn capture is caught by the final capture check
		}
		else if (moving == KING) {
			// Check for castling
			if ((from == E1 && to == G1) || (from == E8 && to == G8)) // King side
				flag = K_CASTLE;
			if ((from == E1 && to == C1) || (from == E8 && to == C8)) // Queen side
				flag = Q_CASTLE;
		}

		if (occupancy[Them] & to_bb)
			flag |= CAPTURE;

		if (!apply_flags)
			// If in debug mode, make sure the flag detected corresponds to the one actually set
			ASSERT(flag == m.flag(), "apply_flags produced flag " << flag << " instead of " << m.flag());
		
		// Modify the old move with new flag
		m = move_t(from, to, flag);
	}

	bool is_ep = (flag == EP_CAPTURE);
	bool is_castle = (flag == K_CASTLE || flag == Q_CASTLE);
	bool is_promo = (flag & 0b1000) != 0;
	bool is_double = (flag == DOUBLE_PUSH);
	bool is_capture = is_ep || (flag & CAPTURE) || (occupancy[Them] & to_bb);

	state_t next = prev;
	next.turn = Them;
	next.ep_square = -1;
	next.captured = NONE;

	if (is_capture && !is_ep) {
		PieceType captured = mailbox[to];
		if (captured != NONE) {
			pieces[captured] &= ~to_bb;
			occupancy[Them] &= ~to_bb;
			next.captured = captured;
		}
	}

	if (is_ep) {
		int cap_sq = to - Up;
		Bitboard cap_bb = 1ULL << cap_sq;
		pieces[PAWN] &= ~cap_bb;
		occupancy[Them] &= ~cap_bb;
		mailbox[cap_sq] = NONE;
		next.captured = PAWN;
	}

	pieces[moving] &= ~from_bb;
	occupancy[Us] &= ~from_bb;
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
		occupancy[Us] |= to_bb;
		mailbox[to] = promo;
	} else {
		pieces[moving] |= to_bb;
		occupancy[Us] |= to_bb;
		mailbox[to] = moving;
	}

	if (is_castle && moving == KING) {
		int rook_from = -1;
		int rook_to = -1;
		if (Us == WHITE) {
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
		occupancy[Us] &= ~rook_from_bb;
		occupancy[Us] |= rook_to_bb;
		mailbox[rook_from] = NONE;
		mailbox[rook_to] = ROOK;
	}

	int rights = prev.castling_rights; // bit format is KQkq (uppercase = WHITE)
	if (moving == KING) {
		if (Us == WHITE)
			rights &= ~((1 << 2) | (1 << 3));
		else
			rights &= ~((1 << 0) | (1 << 1));
	}
	if (moving == ROOK) {
		if (Us == WHITE) {
			if (from == H1) rights &= ~(1 << 3);
			else if (from == A1) rights &= ~(1 << 2);
		} else {
			if (from == H8) rights &= ~(1 << 1);
			else if (from == A8) rights &= ~(1 << 0);
		}
	}
	if (is_capture && !is_ep) {
		if (to == H1) rights &= ~(1 << 3);
		else if (to == A1) rights &= ~(1 << 2);
		else if (to == H8) rights &= ~(1 << 1);
		else if (to == A8) rights &= ~(1 << 0);
	}

	next.castling_rights = rights;
	if (is_double)
		next.ep_square = from + Up;

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
		moving = mailbox[to];
		pieces[moving] &= ~to_bb;
		occupancy[us] &= ~to_bb;
		mailbox[to] = NONE;
	}

	pieces[moving] |= from_bb;
	occupancy[us] |= from_bb;
	mailbox[from] = moving;

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
		mailbox[rook_from] = ROOK;
	}

	if (last.captured != NONE) {
		if (is_ep) {
			int cap_sq = to + (us == WHITE ? -8 : 8);
			Bitboard cap_bb = 1ULL << cap_sq;
			pieces[PAWN] |= cap_bb;
			occupancy[them] |= cap_bb;
			mailbox[cap_sq] = PAWN;
		} else {
			pieces[last.captured] |= to_bb;
			occupancy[them] |= to_bb;
			mailbox[to] = last.captured;
		}
	}

	occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];
}

// Other utilities
std::string board_t::to_fen() const
{
	std::string fen = "";
	const state_t& current = history.back();

	for (int r = 7; r >= 0; --r) {
		int empty = 0;
		for (int f = 0; f < 8; ++f) {
			int sq = r * 8 + f;
			int pc = mailbox[sq];
			if (pc == 0) {
				empty++;
			}
			else {
				if (empty > 0) fen += std::to_string(empty);
				empty = 0;

				// Use offest to transform lowercase to uppercase if white piece
				// Offset is ('A' - 'a') for white piece, 0 for black.
				char offset = ('A' - 'a') * ((occupancy[WHITE] >> sq) & 1ULL);
				switch (pc) {
					case PAWN:
						fen += 'p' + offset; break;
					case KNIGHT:
						fen += 'n' + offset; break;
					case BISHOP:
						fen += 'b' + offset; break;
					case ROOK:
						fen += 'r' + offset; break;
					case QUEEN:
						fen += 'q' + offset; break;
					case KING:
						fen += 'k' + offset; break;
				}
			}
		}
		if (empty > 0) fen += std::to_string(empty);
		if (r > 0) fen += "/";
	}

	// Turn
	fen += (current.turn == WHITE) ? " w " : " b ";

	// Castling
	if (current.castling_rights == 0) fen += "-";
	else {
		if (current.castling_rights & (1 << 3)) fen += "K";
		if (current.castling_rights & (1 << 2)) fen += "Q";
		if (current.castling_rights & (1 << 1)) fen += "k";
		if (current.castling_rights & (1 << 0)) fen += "q";
	}

	// EP
	auto sq_to_string = [&](int sq) {
		return std::string("") + (char)('a' + sq % 8) + (char)('1' + sq / 8); 
	};
	fen += " " + (current.ep_square == -1 ? "-" : sq_to_string(current.ep_square));

	return fen;
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

	int king_sq = std::countr_zero(king_bb);
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
