#include "board.h"

board_t::board_t()
{
	turn = WHITE;

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

	state_t init{0, (1<<4)-1, -1, NONE}; // z_key, castling_rights, ep_square, captured
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
