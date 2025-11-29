#include "board.h"
#include "piece.h"

piece_t* board_t::get_piece(int rank, int file)
{
    return board[rank][file];
}
// implementations of board functions
