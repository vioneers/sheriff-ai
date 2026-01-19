#include <iostream>
#include "opening_book.h"
#include "openings.h"

OpeningBook& get_openings(){
    static OpeningBook openings;
    openings.init_lookup_table();
    return openings;
}

void OpeningBook::init_lookup_table(){
    if (initialized) // ensure initialization runs only once
        return; 
    initialized = true;
    lookup_table.reserve(4096); // upperbound on size to prevent rehashes

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

            // std::cout << move.to_code() << " has flag " << move.flag() << " " << curr_key << '\n';
            
            if (!legal_move_uci(tmp_board, uci, move))
                break; 
            if (lookup_table.find(curr_key) == lookup_table.end())
                lookup_table.emplace(curr_key, move);
            tmp_board.make_move(move);
        }

        // std::cout << '\n';
    }
}

bool OpeningBook::legal_move_uci(board_t& board, std::string& uci, move_t& best_move){
    std::vector<move_t>legal;
    board.get_legal_moves(legal);

    for(auto& move : legal){
        if(move.to_code() == uci){
            best_move = move;
            return true; // the move is legal
        }
    }
    return false; // the move is not legal
}

bool OpeningBook::probe(board_t& board, move_t& best_move) const{
    uint64_t curr_key = board.history.back().z_key;

    auto it = lookup_table.find(curr_key);
    if (it == lookup_table.end()) // we don't have this configuration in the opening book
        return false;
    best_move = it->second; // (key, move) pairs so move is it->second
    return true;
}