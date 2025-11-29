#ifndef __MOVE__
#define __MOVE__

#include <string>

struct move_t {
	size_t from_rank;
	size_t from_file;
	size_t to_rank;
	size_t to_file;
	char promotion; // promotion code if promotion, 0 otherwise
	
	move_t(size_t from_rank, size_t from_file, size_t to_rank, size_t to_file, char promotion = 0)
		: from_rank{from_rank}, from_file{from_file}, to_rank{to_rank}, to_file{to_file}, promotion{promotion} {};
	
	string to_code(); // return the move in long algebraic UCI format
};

move_t move_from_code(const std::string& code);

#endif
