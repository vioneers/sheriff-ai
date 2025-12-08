#ifndef __MOVE__
#define __MOVE__

#include <string>

struct move_t {
	int from_rank;
	int from_file;
	int to_rank;
	int to_file;
	char promotion; // promotion code if promotion, 0 otherwise
	bool is_ep, is_castling;
	char captured;

    move_t()
        : from_rank{0}, from_file{0}, to_rank{0}, to_file{0},
          promotion{0}, is_ep{false}, is_castling{false}, captured{0} {}
	
	move_t(int from_rank, int from_file, int to_rank, int to_file, char promotion = 0)
		: from_rank{from_rank}, from_file{from_file}, to_rank{to_rank}, to_file{to_file}, promotion{promotion} {};
	
	move_t(const std::string& code);
	
	std::string to_code(); // return the move in long algebraic UCI format

    bool operator==(const move_t& other) const {
        return from_rank == other.from_rank &&
               from_file == other.from_file &&
               to_rank == other.to_rank &&
               to_file == other.to_file &&
               promotion == other.promotion;
    }
};

#endif
