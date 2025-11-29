#ifndef __MOVE__
#define __MOVE__

#include <string>

struct move_t {
	size_t from_x;
	size_t from_y;
	size_t to_x;
	size_t to_y;
	char promotion; // promotion code if promotion, 0 otherwise
	
	move_t(size_t from_x, size_t from_y, size_t to_x, size_t to_y, char promotion = 0)
		: from_x{from_x}, from_y{from_y}, to_x{to_x}, to_y{to_y}, promotion{promotion} {};
	
	move_t(string code); // create move from string in long algebraic UCI format
	
	string encode(); // return the move in long algebraic UCI format

}

#endif
