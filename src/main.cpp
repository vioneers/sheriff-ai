#include <fstream> 
#include <iostream>
#include <vector>
#include <random>
#include <chrono> 

#include "utils.h"
#include "move.h"
#include "engine.h"
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
	engine.time_limit = std::chrono::milliseconds(9000);  // ~9 seconds
	engine.time_up = false;

	vector<move_t> legal;
	Color turn = engine.board.history.back().turn;
	engine.board.get_legal_moves(legal);
	if (!legal.empty()){ // In case we're close to a timeout, just output a random legal move for now
		std::mt19937 rng(std::random_device{}());
		std::uniform_int_distribution<int> dist(0, (int)legal.size() - 1);
		engine.best_move = legal[dist(rng)];
		engine.best_move_valid = true;
	}
	else
		engine.best_move_valid = false;

	// if playing White, you are maximizing, else minimize
	// TODO: make it so we always maximize
    int score = 0;
	if(turn == WHITE)
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
