#include "opening_book.h"

const std::vector<std::string> OPENINGS = {
    // Source: https://thechessworld.com/articles/openings/chess-statistics-top-10-best-openings-for-white-and-black/
    // BEST FOR WHITE
    // Queen’s Gambit
    "d2d4 d7d5 c2c4",
	// Blackmar Diemer Gambit
    "d2d4 d7d5 e2e4 d5e4",
	// Ruy Lopez
    "e2e4 e7e5 g1f3 b8c6 f1b5",
	// Bishop’s Opening
    "e2e4 e7e5 f1c4",
	// Benko Opening
    "c2c4 g8f6 b1c3",
	// Reti Opening
    "g1f3 d7d5 c2c4",
	// Vienna Game
    "e2e4 e7e5 b1c3",
	// Centre Game
    "e2e4 e7e5 d2d4",
	// English Opening
    "c2c4 e7e5 b1c3",
	// Scotch Game
    "e2e4 e7e5 g1f3 b8c6 d2d4",

    //BEST FOR BLACK
    // Sicilian Defense
    "e2e4 c7c5",
	// Nimzo Indian
    "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4",
	// Robatsch Defense
    "e2e4 g7g6 d2d4 f8g7",
	// Alekhine Defense
    "e2e4 g8f6",
	// Nimzowitsch Defense
    "e2e4 b8c6",
	// Rat
    "d2d4 d7d6 c2c4 g8f6",
	// Benko Gambit
    "d2d4 g8f6 c2c4 c7c5 d4c5 b7b5",
	// Modern Defense
    "e2e4 g7g6 d2d4 f8g7",
	// Queen’s Indian Defense
    "d2d4 g8f6 c2c4 e7e6 g1f3 b7b6",
	// Pseudo King’s Indian
    "d2d4 g8f6 c2c4 g7g6"
};

void OpeningBook::init_lookup_table(){
    for (const std::string& opening_line : OPENINGS){
        board_t tmp_board;
        int n = (int)opening_line.size();
        int left = 0;
        while (left < n){
            int right = left; 
            while (right < n && opening_line[right] != ' ')
                right++;
            std::string uci = opening_line.substr(left, right - left);
            left = right + 1; // go to the next move, skip the blank space

            uint64_t curr_key = tmp_board.history.back().z_key;
            move_t move; 
            if (!legal_move_uci(tmp_board, uci, move))
                break;
            if (lookup_table.find(curr_key) == lookup_table.end())
                lookup_table.emplace(curr_key, move);
            tmp_board.make_move(move);
        }
    }
}

bool OpeningBook::legal_move_uci(board_t& board, std::string& uci, move_t& best_move){
    std::vector<move_t>legal;
    board.get_legal_moves(legal);

    for(auto& move : legal){
        if(move.to_code() == uci){
            best_move = uci;
            return true; // the move is legal
        }
    }
    return false; // the move is not legal
}

bool OpeningBook::probe(board_t& board, move_t& best_move) const{
    uint64_t curr_key = board.history.back().z_key;

    auto it = lookup_table.find(curr_key);
    best_move = it->second; // (key, move) pairs so move is it->second
    if (it == lookup_table.end()) // we don't have this configuration in the opening book
        return false;
    return true;
}