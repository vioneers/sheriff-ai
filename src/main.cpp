#include <fstream> 
#include <iostream>

#include "utils.h"
#include "move.h"

using namespace std; 

int main(int argc, char* argv[])
{
	// parse arguments and make sure both filenames are given
	auto [in_file_name, out_file_name] = parse_args(argc, argv);
	
	if(in_file_name == "")
	{
		cout << "Input file not provided. Pass it with -H" << endl;
		return 1;
	}
	if(out_file_name == "")
	{
		cout << "Output file not provided. Pass it with -m" << endl;
		return 1;
	}
	
	vector<move_t> move_hist = get_move_history(in_file_name);

	// run fancy algorithm

	vector<move_t> open_w = {{"e2e4"}, {"d1h5"}, {"f1c4"}, {"h5f7"}};
	vector<move_t> open_b = {{"e7e6"}, {"a7a6"}, {"d8h4"}, {"h4g3"}, {"g3f2"}};

	move_t move{"d7d5"}; // random move
	int turn = move_hist.size();
	if(turn % 2 == 0)
		move = open_w[turn/2];
	else
		move = open_b[turn/2]; // pro gamer move
	
	write_move(move, out_file_name);	

    return 0;
}
