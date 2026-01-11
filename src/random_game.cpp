#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <random>
#include <thread>

namespace fs = std::filesystem;

#include "types.h"
#include "engine.h"
#include "utils.h"

using namespace std;

#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>

std::string generate_pgn(const engine_t& engine, const std::vector<move_t>& history, std::string result, Color ourColor) {
	std::stringstream pgn;

	// 1. Mandatory Header Tags
	pgn << "[Event \"Engine Random Match\"]\n";
	pgn << "[Site \"Local Machine\"]\n";

	// Get current date
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	pgn << "[Date \"" << std::put_time(&tm, "%Y.%m.%d") << "\"]\n";

	pgn << "[Round \"1\"]\n";
	pgn << "[White \"" << (ourColor == WHITE ? "OurBot" : "RandomBot") << "\"]\n";
	pgn << "[Black \"" << (ourColor == BLACK ? "OurBot" : "RandomBot") << "\"]\n";
	pgn << "[Result \"" << result << "\"]\n\n";

	// 2. Move List
	for (size_t i = 0; i < history.size(); ++i) {
		// Every two moves (at the start of White's turn), print the move number
		if (i % 2 == 0) {
			pgn << (i / 2 + 1) << ". ";
		}

		// Use UCI code. Note: Standard PGN prefers SAN, 
		// but most readers accept UCI if SAN isn't available.
		char pieceChar;
		switch (engine.board.mailbox[history[i].to()])
		{
		case PAWN:   pieceChar = 'P'; break;
		case KNIGHT: pieceChar = 'N'; break;
		case BISHOP: pieceChar = 'B'; break;
		case ROOK:   pieceChar = 'R'; break;
		case QUEEN:  pieceChar = 'Q'; break;
		case KING:   pieceChar = 'K'; break;
		}
		pgn << pieceChar << history[i].to() << " ";

		// Optional: Line break every 10 plies for readability
		if ((i + 1) % 10 == 0) pgn << "\n";
	}

	// 3. Final Result
	pgn << result << "\n";

	return pgn.str();
}

move_t play_random_bot(engine_t &engine) {
	vector<move_t> legal;
	engine.board.get_legal_moves(legal);

	if (legal.empty())
		return move_t{}; // if no legal moves just return null move 

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<std::size_t> dist(0, legal.size() - 1);

	return legal[dist(gen)];
}

move_t play_our_bot(engine_t& engine)
{
	using clock = std::chrono::steady_clock;
	engine.start_time = clock::now();
	engine.time_limit = std::chrono::milliseconds(9500);  // ~9.5 seconds
	engine.time_up = false;

	engine.root_best_move = move_t{}; // engine assumes it starts with null move

	// if playing White, you are maximizing, else minimize
	int score = 0;
	Color turn = engine.board.history.back().turn;
	if (turn == WHITE)
		score = engine.alphaBetaMax(-1e9, 1e9, engine_t::DEFAULT_SEARCH_DEPTH);
	else
		score = engine.alphaBetaMin(-1e9, 1e9, engine_t::DEFAULT_SEARCH_DEPTH);

	return engine.root_best_move;
}

void save_game_log(engine_t& engine, Color Us, int game_id, std::string folder, vector<move_t>& game_history, int fifty_cnt)
{
	// 1. Create a unique filename for this specific game
	std::string filename = folder + "/game_" + std::to_string(game_id) + ".txt";
	std::ofstream outfile(filename);

	// pgn doesnt work yet
	/*std::string pgnname = folder + "/game_" + std::to_string(game_id) + ".pgn";
	std::ofstream pgnfile(pgnname);*/

	if (!outfile.is_open()) {
		return; // Handle error: could not open file
	}

	std::string final_result = "*"; // Default to "unfinished"

	for (auto& m : game_history)
		outfile << m.to_code() << '\n';

	if (engine.board.history.size() >= 300)
		outfile << "game timed out" << std::endl;
	else if (game_history.back().is_null())
	{
		Color turn = engine.board.history.back().turn;
		std::string color = (~turn == WHITE) ? "WHITE" : "BLACK";
		
		final_result = (turn == WHITE) ? "0-1" : "1-0";

		if (engine.board.in_check(turn))
			outfile << color << " wins!" << '\n';
		else
			outfile << "stalemate" << '\n';
	} 
	else if (engine.board.is_repetition(3))
	{
		outfile << "draw (threefold repetition)" << '\n';
		final_result = "1/2-1/2";
	}
	else if (fifty_cnt <= 0)
	{
		outfile << "draw (fifty-move rule)" << '\n';
		final_result = "1/2-1/2";
	}

	//pgnfile << generate_pgn(engine, game_history, final_result, Us);
}

void play_against_random(engine_t engine, Color Us, int game_id, std::string folder)
{
	std::vector<move_t> game_history;
	int fifty_cnt = 100;

	while (engine.board.history.size() < 300)
	{
		Color turn = engine.board.history.back().turn;

		move_t last_move = (turn == Us) ? play_our_bot(engine) : play_random_bot(engine);


		game_history.push_back(last_move);
		if (last_move.is_null())
			break;

		engine.board.make_move(last_move);
		fifty_cnt--;

		if ((last_move.flag() & CAPTURE) || (engine.board.mailbox[last_move.to()] == PAWN))
			fifty_cnt = 100;

		if (engine.board.is_repetition(3) || fifty_cnt <= 0)
			break;
	}

	save_game_log(engine, Us, game_id, folder, game_history, fifty_cnt);
}

void run_multithreaded_games(int id1, int id2, engine_t &engine_instance, Color Us) {
	std::string folder = "game_logs";
	fs::create_directory(folder); // Ensure folder exists

	std::vector<std::thread> threads;

	for (int i = id1; i < id2; ++i) {

		// Launch a thread. Each thread gets its own engine copy and unique ID.
		threads.emplace_back(play_against_random, engine_instance, Us, i, folder);
	}

	// Join threads (wait for all games to finish)
	for (auto& t : threads) {
		if (t.joinable()) {
			t.join();
		}
	}

	std::cout << "All " << id2-id1 << " games completed." << std::endl;
}

int main(int argc, char* argv[])
{
	vector<move_t> move_hist;
	engine_t engine(move_hist);
	// id2 - id1 games
	run_multithreaded_games(0, 5, engine, WHITE);

	return 0;
}