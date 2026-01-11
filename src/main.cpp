#include <fstream> 
#include <iostream>
#include <vector>
#include <random>
#include <chrono> 

#include "utils.h"
#include "move.h"
#include "engine.h"
#include "board.h"
#include "zobrist.h"
#include "opening_book.h"
#include "debug.h"

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
	engine.time_limit = std::chrono::milliseconds(9500);  // ~9.5 seconds
	engine.time_up = false;

    move_t best_move = engine.get_best_move();

    if(best_move.is_null()){
        std::ofstream outFile(out_file_name);
        outFile.close(); // leave empty to signal no legal moves
        return 0;
    }

#ifdef DEBUG
	print_debug_info(engine);
#endif

	write_move(best_move, out_file_name);

    return 0;
}


// Build with "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release"

// FOR DEBUGGING: "cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build --config Debug"

// Run with "./build/sheriff_ai -H input.txt -m output.txt"
