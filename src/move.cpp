#include <string>
#include "move.h"

move_t::move_t(const std::string& code)
{
	from_rank = code[1] - '1';
	from_file = code[0] - 'a';
	to_rank = code[3] - '1';
	to_file = code[2] - 'a';
    promotion = 0;
	if(code.size() == 5)
		promotion = code[4];
}

std::string move_t::to_code()
{
	std::string from = std::string("") + (char)('a' + from_file) + (char)('1' + from_rank);
	std::string to = std::string("") + (char)('a' + to_file) + (char)('1' + to_rank);
	return from + to + promotion;
}


