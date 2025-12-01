#include "piece.h"
#include "board.h"

std::vector<move_t> king_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    const int directions[8][2] = {
        {  1,  0 }, { -1,  0 }, {  0,  1 }, {  0, -1 },
        {  1,  1 }, {  1, -1 }, { -1,  1 }, { -1, -1 }
    };

    for (int i = 0; i < 8; i++){
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1]; 

        // Don't go outside the board
        if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7)
            continue; 
        
        // See if there is already a piece in the destination 
        piece_t* destination = board->get_piece(new_rank, new_file);

        // Destination is empty => possible move OR Can capture the opponent's piece 
        if (destination == nullptr || destination->color != this->color)
            moves.emplace_back(rank, file, new_rank, new_file);
    }

    return moves;
}

std::vector<move_t> knight_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    const int directions[8][2] = {
        {  2,  1 }, {  2, -1 }, { -2,  1 }, { -2, -1 },
        {  1,  2 }, {  1, -2 }, { -1,  2 }, { -1, -2 }
    };

    for (int i = 0; i < 8; i++){
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1]; 

        // Don't go outside the board
        if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7)
            continue; 
        
        // See if there is already a piece in the destination 
        piece_t* destination = board->get_piece(new_rank, new_file);

        // Destination is empty => possible move OR Can capture the opponent's piece 
        if (destination == nullptr || destination->color != this->color)
            moves.emplace_back(rank, file, new_rank, new_file);
    }

    return moves;
}

std::vector<move_t> rook_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    const int directions[4][2] = {
        {  1,  0 }, { -1,  0 }, {  0,  1 }, {  0, -1 }
    };

    for (int i = 0; i < 4; i++){
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1]; 

        // Slide until outside the board (or until break reached)
        while (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            // See if there is already a piece in the destination 
            piece_t* destination = board->get_piece(new_rank, new_file);

            // Destination is empty => possible move 
            if (destination == nullptr)
                moves.emplace_back(rank, file, new_rank, new_file);
            else{
                if (destination->color != this->color) // Capture opponent's piece
                    moves.emplace_back(rank, file, new_rank, new_file);
                break; // If we captured or can no longer pass, we stop
            }

            new_rank += directions[i][0];
            new_file += directions[i][1]; 
        }
    }

    return moves;
}

std::vector<move_t> bishop_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    const int directions[4][2] = {
        {  1,  1 }, {  1, -1 }, { -1,  1 }, { -1, -1 } 
    };

    for (int i = 0; i < 4; i++){
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1]; 

        // Slide until outside the board (or until break reached)
        while (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            // See if there is already a piece in the destination 
            piece_t* destination = board->get_piece(new_rank, new_file);

            // Destination is empty => possible move 
            if (destination == nullptr)
                moves.emplace_back(rank, file, new_rank, new_file);
            else{
                if (destination->color != this->color) // Capture opponent's piece
                    moves.emplace_back(rank, file, new_rank, new_file);
                break; // If we captured or can no longer pass, we stop
            }

            new_rank += directions[i][0];
            new_file += directions[i][1]; 
        }
    }

    return moves;
}

std::vector<move_t> queen_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    // Directions of queen are directions of rook + bishop
    const int directions[8][2] = {
        {  1,  0 }, { -1,  0 }, {  0,  1 }, {  0, -1 }, {  1,  1 }, {  1, -1 }, { -1,  1 }, { -1, -1 } 
    };

    for (int i = 0; i < 8; i++){
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1]; 

        // Slide until outside the board (or until break reached)
        while (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            // See if there is already a piece in the destination 
            piece_t* destination = board->get_piece(new_rank, new_file);

            // Destination is empty => possible move 
            if (destination == nullptr)
                moves.emplace_back(rank, file, new_rank, new_file);
            else{
                if (destination->color != this->color) // Capture opponent's piece
                    moves.emplace_back(rank, file, new_rank, new_file);
                break; // If we captured or can no longer pass, we stop
            }

            new_rank += directions[i][0];
            new_file += directions[i][1]; 
        }
    }

    return moves;
}

std::vector<move_t> pawn_t :: get_available_moves(board_t* board, int rank, int file){
    std::vector<move_t> moves;
    int dir, initial_rank; // dir means dir_rank
    if (this->color == false) {   // White => rank up
        dir = -1;                 
        initial_rank = 6;
    } else {                      // Black => rank down
        dir = 1;                  
        initial_rank = 1;
    }

    // Forward move (1 step or can be 2 steps if started at initial_rank)
    int new_rank = rank + dir;
    // Don't go outside the board
    if (new_rank >= 0 && new_rank <= 7){
        piece_t* destination = board->get_piece(new_rank, file);

        // Destination is empty => possible move 
        if (destination == nullptr){
            moves.emplace_back(rank, file, new_rank, file);
            // Can move 2 steps if we come from the starting rank
            if (rank == initial_rank){
                new_rank += dir;
                if (new_rank >= 0 && new_rank <= 7){
                    piece_t* destination = board->get_piece(new_rank, file);
                    if (destination == nullptr)
                        moves.emplace_back(rank, file, new_rank, file);
                }
            }
        }
    }

    // Diagonal move allowed only if capture
    int dir_file[2] = {-1, 1}; 
    for (int i = 0; i < 2; i++){
        int new_rank = rank + dir;
        int new_file = file + dir_file[i];
        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <=7)
        {
            piece_t* destination = board->get_piece(new_rank, new_file);
            // Need opponent piece, not empty
            if (destination != nullptr && destination->color != this->color)
                moves.emplace_back(rank, file, new_rank, new_file);
        }
    }
    return moves;
}