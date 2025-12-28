#pragma once

#include <string>
#include <vector>
#include "types.h"
#include "move.h"

struct state_t {
    uint64_t z_key;          // Zobrist hash for Transposition Table //TODO
    int castling_rights;     // Bitmask (4 bits for KQkq)
    int ep_square;           // En passant target square
    PieceType captured;			 // last piece to be captured
	Color turn; // BLACK / WHITE
};

struct board_t {
    Bitboard pieces[7];   // [PieceType], [0] is unused.
    Bitboard occupancy[3];   // [White, Black, Both]
    PieceType mailbox[64]; // Get pieces by board position (to get color use occupancy[WHITE])

    // History stack for unmake_move
    std::vector<state_t> history;

	board_t(); // default constructor - initial position
	board_t(std::string fen);
	board_t(std::vector <move_t> move_hist);

	// check detection
	bool square_attacked(int sq, Color by_color) const;
	bool in_check(Color by_color) const;

	// generate legal moves for the side to move and add to list
	void get_legal_moves(std::vector<move_t> &list);

	// verify that move is actually legal and add to list
	void add_move(std::vector<move_t> &list, move_t move);

	// get pseudolegal moves (implemented in pieces.cpp)
	template<Color Us> void generate_pawn_moves(std::vector<move_t> &list);
    template<Color Us> void generate_knight_moves(std::vector<move_t> &list);
	template<Color Us> void generate_king_moves(std::vector<move_t> &list);

	template<Color Us> void generate_queen_moves(std::vector<move_t> &list);
	template<Color Us> void generate_rook_moves(std::vector<move_t> &list);
    template<Color Us> void generate_bishop_moves(std::vector<move_t> &list);

	// apply_flags - Whether to make extra checks to apply the right flags to the move object. Used when applying moves converted from UCI
	// return False if fail
    bool make_move(move_t& m, bool apply_flags = false);
    void undo_move(move_t m);
	
	// other utils
	std::string to_fen() const;
};

// Include implementations of template functions at the bottom
#include "movegen.h"