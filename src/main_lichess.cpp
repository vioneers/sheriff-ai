#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "utils.h"
#include "move.h"
#include "engine.h"
// #include "piece.h"
#include "board.h"

using namespace std;

static string compute_bestmove(const vector<move_t>& move_hist) {
    engine_t engine(move_hist);

    using clock = std::chrono::steady_clock;
    engine.start_time = clock::now();
    engine.time_limit = std::chrono::milliseconds(9800); // ~9.8 seconds
    engine.time_up = false;

    if(move_hist.size() % 2 == 0)
		engine.alphaBetaMax(-1e9, 1e9, 6);
	else
		engine.alphaBetaMin(-1e9, 1e9, 6);

    if(!engine.best_move_valid)
        return "0000"; // UCI “no move” sentinel for terminal nodes
    return engine.best_move.to_code();
}

int main() {
    vector<move_t> move_hist;
    string line;
    cout.setf(std::ios::unitbuf); // auto-flush

    while (getline(cin, line)) {
        if (line == "uci") {
            cout << "id name Sheriff-AI\n";
            cout << "id author sheriff-ai\n";
            cout << "uciok\n";
        } else if (line == "isready") {
            cout << "readyok\n";
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
                // Minimal stub: treat as startpos and apply trailing moves
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
    return 0;
}
