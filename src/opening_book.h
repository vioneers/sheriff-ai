#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "board.h"

struct OpeningBook{
    std::unordered_map<uint64_t, move_t> lookup_table;
    bool initialized = false;
    void init_lookup_table();
    bool probe(board_t& board, move_t& best_move) const;
    bool legal_move_uci(board_t& board, std::string& uci, move_t& best_move);
};

OpeningBook& get_openings();