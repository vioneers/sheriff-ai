#include <fstream> 
#include <iostream>
#include <vector>

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

	bool turn; 
	if (move_hist.size() % 2 == 0) 
		turn = false; // White
	else
		turn = true; // Black

	vector <move_t> legal; // vector that will store all legal moves
	for (int rank = 0; rank < 8; rank++){
		for (int file = 0; file < 8; file++){
			piece_t* p = engine.board_state.get_piece(rank,file); 
			if (p!= nullptr && p->color == turn){ // If the current piece if the color of the current turn
				// Start with pseudo_legal moves returned by get_available_moves (method of piece_t)
				vector <move_t> pseudo_legal = p->get_available_moves(&engine.board_state, rank, file); 
				for(auto& move : pseudo_legal){ // Check all pseudo_legal moves in the current configuration of the board (method of board_t)
					if(engine.board_state.check_move(&move))
						legal.push_back(move);
				}
			}
		}
	}

	// For this stage of the project, we choose the first legal move.
	if (!legal.empty()) // There exists at least one legal move to make
		write_move(legal[0], out_file_name); 
    return 0;
}
