#include <string>
#include "move.h"

move_t::move_t(const std::string& code)
{
	from_file = code[0] - 'a';
	to_file = code[2] - 'a';
	from_rank = 8 - (code[1] - '0');
    to_rank   = 8 - (code[3] - '0');
    promotion = 0;
	if(code.size() == 5)
		promotion = code[4];
}

std::string move_t::to_code()
{
	std::string from = std::string("") + (char)('a' + from_file) + (char)('0' + (8 - from_rank));
	std::string to = std::string("") + (char)('a' + to_file) + (char)('0' + (8 - to_rank));
	if (promotion != 0) // Fix to not have a NULL at the end of the line if no promotion
		return from + to + promotion;
	return from + to; 
}


