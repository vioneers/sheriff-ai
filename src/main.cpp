#include <fstream> 
#include <iostream>
#include <vector>
#include <random>

#include "utils.h"
#include "move.h"
#include "engine.h"
#include "piece.h"
#include "board.h"

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
	engine_t engine(move_hist);

	vector <move_t> legal = engine.board_state.get_legal_moves(); // vector that will store all legal moves

	// For this stage of the project, we choose the first legal move.
	if (!legal.empty()) // There exists at least one legal move to make
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
        write_move(legal[dist(rng)], out_file_name);
    }

    return 0;
}
