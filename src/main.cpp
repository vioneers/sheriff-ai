#include <fstream> 
#include <iostream>
#include <vector>
#include <random>
#include <chrono> 

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
	
	using clock = std::chrono::steady_clock;
	engine.start_time = clock::now();
	engine.time_limit = std::chrono::milliseconds(9800);  // ~9.8 seconds
	engine.time_up = false;

	// if playing White, you are maximizing, else minimize
    int score = 0;
	if(move_hist.size() % 2 == 0)
		score = engine.alphaBetaMax(-1e9, 1e9, 5);
	else
		score = engine.alphaBetaMin(-1e9, 1e9, 5);
    // cout << score;

    if(!engine.best_move_valid){
        std::ofstream outFile(out_file_name);
        outFile.close(); // leave empty to signal no legal moves
        return 0;
    }
	write_move(engine.best_move, out_file_name);

    return 0;
}


// Build with "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release"
