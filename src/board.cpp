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

void make_move(move_t m){
	// TODO:
	// make move
	// update mailbox
	// push to params stack
	// update flags in move object so that we can then use them for unmake move?
}

void undo_move(move_t m)
{
	// TODO:
	// pop from params stack
	// unmake move
	// update mailbox
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
