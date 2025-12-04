#include <fstream> 
#include <iostream>
#include <vector>
#include <sstream>

#include "utils.h"
#include "move.h"
#include "engine.h"
#include "piece.h"
#include "board.h"

using namespace std; 

static string compute_bestmove(const vector<move_t>& move_hist);
static void uci_loop();

int main(int argc, char* argv[])
{
	// parse arguments and make sure both filenames are given
	auto [in_file_name, out_file_name] = parse_args(argc, argv);

	// If no args provided, run in UCI mode
    if (in_file_name.empty() && out_file_name.empty()) {
        uci_loop();
        return 0;
    }
	
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
		write_move(legal[0], out_file_name); 

    return 0;
}

static string compute_bestmove(const vector<move_t>& move_hist)
{
    engine_t engine(move_hist);

    bool turn = (move_hist.size() % 2 != 0); // false = White, true = Black

    vector<move_t> legal;
    for (int rank = 0; rank < 8; rank++){
        for (int file = 0; file < 8; file++){
            piece_t* p = engine.board_state.get_piece(rank,file); 
            if (p!= nullptr && p->color == turn){
                vector<move_t> pseudo_legal = p->get_available_moves(&engine.board_state, rank, file); 
                for(auto& mv : pseudo_legal){
                    if(engine.board_state.check_move(&mv))
                        legal.push_back(mv);
                }
            }
        }
    }
	
    if (legal.empty())
        return "0000";
    return legal[0].to_code();
}

static void uci_loop()
{
    vector<move_t> move_hist;
    string line;
    cout.setf(std::ios::unitbuf); // auto-flush

    while (std::getline(cin, line)) {
        if (line == "uci") {
            cout << "id name Sheriff-AI" << '\n';
            cout << "id author sheriff-ai" << '\n';
            cout << "uciok" << '\n';
        } else if (line == "isready") {
            cout << "readyok" << '\n';
        } else if (line == "ucinewgame") {
            move_hist.clear();
        } else if (line.rfind("position", 0) == 0) {
            auto tokens = split_tokens(line);
            if (tokens.size() >= 2 && tokens[1] == "startpos") {
                move_hist.clear();
                for (size_t i = 2; i < tokens.size(); ++i) {
                    if (tokens[i] == "moves") {
                        append_moves_from_tokens(move_hist, tokens, i + 1);
                        break;
                    }
                }
            } else if (tokens.size() >= 2 && tokens[1] == "fen") {
                // Minimal stub: we do not parse FEN, treat as startpos and read trailing moves if any
                move_hist.clear();
                for (size_t i = 0; i < tokens.size(); ++i) {
                    if (tokens[i] == "moves") {
                        append_moves_from_tokens(move_hist, tokens, i + 1);
                        break;
                    }
                }
            }
        } else if (line.rfind("go", 0) == 0) {
            string best = compute_bestmove(move_hist);
            cout << "bestmove " << best << '\n';
        } else if (line == "stop") {
            // No background search; ignore
        } else if (line == "quit") {
            break;
        }
    }
}
