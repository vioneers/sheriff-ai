#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "utils.h"
#include "move.h"
#include "engine.h"
#include "board.h"

using namespace std;

static string compute_bestmove(const vector<move_t>& move_hist, const string& fen) {
    engine_t engine(move_hist);
    if (!fen.empty()) {
        board_t board(fen);
        for (auto move : move_hist) {
            board.make_move(move, true);
        }
        engine.board = board;
    }

    using clock = std::chrono::steady_clock;
    engine.start_time = clock::now();
    engine.time_limit = std::chrono::milliseconds(9500); // ~9.5 seconds
    engine.time_up = false;

    Color turn = engine.board.history.back().turn;
    if (turn == WHITE)
        engine.alphaBetaMax(-1e9, 1e9, engine_t::DEFAULT_SEARCH_DEPTH);
    else
        engine.alphaBetaMin(-1e9, 1e9, engine_t::DEFAULT_SEARCH_DEPTH);

    if(!engine.best_move_valid)
        return "0000"; // UCI “no move” sentinel for terminal nodes
#ifdef SHERIFF_DEBUG_PV
    engine.log_root_lines();
#endif
    return engine.best_move.to_code();
}

int main() {
    vector<move_t> move_hist;
    string position_fen;
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
            position_fen.clear();
        } else if (line.rfind("position", 0) == 0) {
            auto tokens = split_tokens(line);
            if (tokens.size() >= 2 && tokens[1] == "startpos") {
                move_hist.clear();
                position_fen.clear();
                for (size_t i = 2; i < tokens.size(); ++i) {
                    if (tokens[i] == "moves") {
                        append_moves_from_tokens(move_hist, tokens, i + 1);
                        break;
                    }
                }
            } else if (tokens.size() >= 2 && tokens[1] == "fen") {
                move_hist.clear();
                position_fen.clear();
                size_t fen_start = 2;
                size_t fen_end = fen_start;
                for (; fen_end < tokens.size(); ++fen_end) {
                    if (tokens[fen_end] == "moves")
                        break;
                    if (!position_fen.empty())
                        position_fen += " ";
                    position_fen += tokens[fen_end];
                }
                for (size_t i = fen_end; i < tokens.size(); ++i) {
                    if (tokens[i] == "moves") {
                        append_moves_from_tokens(move_hist, tokens, i + 1);
                        break;
                    }
                }
            }
        } else if (line.rfind("go", 0) == 0) {
            string best = compute_bestmove(move_hist, position_fen);
            cout << "bestmove " << best << '\n';
        } else if (line == "stop") {
            // No background search; ignore
        } else if (line == "quit") {
            break;
        }
    }
    return 0;
}
