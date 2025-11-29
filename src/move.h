#ifndef __MOVE__
#define __MOVE__

#include <string>

struct move_t {
	int from_rank;
	int from_file;
	int to_rank;
	int to_file;
	char promotion; // promotion code if promotion, 0 otherwise
	
	move_t(int from_rank, int from_file, int to_rank, int to_file, char promotion = 0)
		: from_rank{from_rank}, from_file{from_file}, to_rank{to_rank}, to_file{to_file}, promotion{promotion} {};
	
	move_t(const std::string& code);
	
	std::string to_code(); // return the move in long algebraic UCI format
};

#endif
