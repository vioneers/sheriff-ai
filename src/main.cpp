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
	
	// if playing White, you are maximizing, elese minimize
	// TODO: make it so we always maximize
	if(move_hist.size() % 2 == 0)
		cout << engine.alphaBetaMax(-1e9, 1e9, 5);
	else
		cout << engine.alphaBetaMin(-1e9, 1e9, 5);
	write_move(engine.best_move, out_file_name);

    return 0;
}


// Build with "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release"
