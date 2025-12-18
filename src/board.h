#ifndef __BOARD__
#define __BOARD__
#include <vector> 
#include <array>
#include <cstdint>

struct move_t;
struct piece_t;

struct params_t {
	bool WK_castle;
	bool WQ_castle;
	bool BK_castle;
	bool BQ_castle;

	// if a pawn that moved last turn is vulnerable to en passant, store the position where an oposing pawn should move to perform en passant.
	int ep_rank;
	int ep_file;

	// was en passant played
	bool ep_played;	

	// hold the piece captured at each turn.
	piece_t* captured;
};

struct board_t {
	piece_t* board[8][8];
	bool turn; // White = false; Black = true

	std::vector <params_t> param_stack;
	std::array <piece_t*, 12> pieces;

	uint64_t key;
	std::vector<uint64_t> key_history;

	uint64_t compute_key() const;

	//bool WK_castle;
	//bool WQ_castle;
	//bool BK_castle;
	//bool BQ_castle;

	// if a pawn that moved last turn is vulnerable to en passant, store the position where an oposing pawn should move to perform en passant.
	//int ep_rank;
	//int ep_file;
	
	board_t();
	board_t(std::array <piece_t*, 12> pieces); 
	board_t(std::vector <move_t> move_hist, std::array <piece_t*, 12> pieces); 

	bool check_move(const move_t move);
	piece_t* get_piece(int rank, int file) const;

	bool check_in_between(int fr, int ff, int tr, int tf) const;
	bool square_attacked(int r, int f, bool by_color) const;
	bool in_check(bool color) const;
	bool is_threefold() const; 

	std::vector <move_t> get_legal_moves();

	void make_move(const move_t move);
	void undo_move(const move_t move);
};

#endif
