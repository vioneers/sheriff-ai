#include <vector>
#include <iostream>
#include "engine.h"

// Perft (performance test, move path enumeration) algorithm to check that we generate all legal moves
unsigned long long perft(int depth, board_t board){
    std::vector <move_t> legal;
    board.get_legal_moves(legal);
    unsigned long long nodes = 0;
    if (depth == 0) 
        return 1ULL;
    for (auto& move : legal){
        board.make_move(move); 
        nodes += perft(depth - 1, board);
        board.undo_move(move); 
    }
    return nodes;
}

int main(){
    std::vector<move_t> move_hist; // empty vector 
    engine_t engine(move_hist);
    board_t board = engine.board;
    for (int i = 0 ; i < 10 ; i++)
        std::cout << perft(i, board) << '\n'; 
    return 0;
}