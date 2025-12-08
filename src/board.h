#pragma once

#include <string>
#include "types.h"

struct state_t {
    uint64_t z_key;          // Zobrist hash for Transposition Table //TODO
    int castling_rights;     // Bitmask (4 bits for KQkq)
    int ep_square;           // En passant target square
    int captured;			 // last piece to be captured
};

struct board_t {
    Bitboard pieces[6];   // [PieceType]
    Bitboard occupancy[3];   // [White, Black, Both]
    PieceType mailbox[64]; // Get pieces by board position (positive values for WHITE, negative for BLACK)
	Color turn; // BLACK / WHITE

    // History stack for unmake_move
    std::vector<GameState> history;

	board_t(std::string fen);
	board_t(std::vector <move_t> move_hist);

	

	// generate moves and upadte move list
	void get_legal_moves();
	void add


	// get pseudolegal moves
	

    bool make_move(move_t m); // return False if fail
    void undo_move(move_t m);
	
	// other utils
	std::string to_fen() const;
};
