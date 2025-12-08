#include <string>
#include "move.h"

move_t::move_t(const std::string& code)
{
    int flags = QUIET; // Since we don't have access to the board state, assume move is quiet
	
	int to_rank = code[3] - '1';
	int to_file = code[2] - 'a';
	int to_sq   = to_rank*8 + to_file; 

	int from_rank = code[1] - '1';
	int from_file = code[0] - 'a';
	int from_sq   = from_rank*8 + from_file;

	if(code.size() == 5)
		switch (code[4])
		{
            case 'q':
                return PROMO_Q;
            case 'r':
                return PROMO_R;
            case 'b':
                return PROMO_B;
            case 'n':
                return PROMO_N;
        }
}

std::string move_t::to_code()
{
	int from_sq = from();
	int to_sq = to();

	std::string from = std::string("") + (char)('a' + from_sq%8) + (char)('1' + from_sq/8);
	std::string to = std::string("") + (char)('a' + to_sq%8) + (char)('1' + to_sq/8);

	std::string code = from + to
	
	switch(flag())
	{
		case PROMO_Q:
			return code += "q";
		case PROMO_R:
			return code += "r";
		case PROMO_B:
			return code == "b";
		case PROMO_N:
			return code += "n";
	}
	
	return code;
}


