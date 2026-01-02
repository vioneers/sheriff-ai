#include <vector>
#include <iostream>
#include "engine.h"
int castle, capture, en_passant, promotion;
// Perft (performance test, move path enumeration) algorithm to check that we generate all legal moves
unsigned long long perft(int depth, board_t board){
    std::vector <move_t> legal;
    board.get_legal_moves(legal);
    unsigned long long nodes = 0;
    if (depth == 0) 
        return 1ULL;
    for (auto& move : legal){
        if (depth == 1){
            switch(move.flag()){
                case K_CASTLE: castle++; break;
                case Q_CASTLE: castle++; break;
                case EP_CAPTURE: en_passant++; break;
                default: break;
            }
            if (move.flag() & PROMO_N)
                promotion++; 
            if (move.flag() & CAPTURE)
                capture++; 
        }
        board.make_move(move); 
        nodes += perft(depth - 1, board);
        board.undo_move(move); 
    }
    return nodes;
}

int main(){
    /*std::vector<move_t> move_hist; // empty vector 
    engine_t engine(move_hist);
    board_t board = engine.board;*/
    board_t board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    std::cout << board.to_fen() << '\n';
    for (int i = 0 ; i < 10 ; i++){
        castle = capture = en_passant = promotion = 0;
        std::cout << perft(i, board) << '\n';
        std::cout << castle << " " << capture << " " << en_passant << " " << promotion << '\n';
    }
    /*std::vector <move_t> legal;
    Color turn = board.history.back().turn;
    board.get_legal_moves(legal, turn);
    for (auto& move : legal)
        std::cout << move.to_code() << '\n';*/
    /*int nodes = perft(3, board);
    std::cout << castle << " " << capture << " " << en_passant << " " << promotion << '\n';*/
    return 0;
}