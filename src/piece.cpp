#include "piece.h"
#include "board.h"

std::vector<move_t> king_t :: get_available_moves(board_t* board, int rank, int file){
	// get latest board params
	params_t params = board->param_stack.back();

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

    piece_t* king = board->get_piece(rank, file);
    // Castling moves
    piece_t* rook;
    if (!king->color){ // White king 
        // King-side
        rook = board->get_piece(7, 7);
        if (params.WK_castle &&
            rook && rook->symbol == 'R' && !rook->color &&
            board->get_piece(7,5) == nullptr &&
            board->get_piece(7,6) == nullptr &&
            !board->in_check(false) &&
            !board->square_attacked(7,5,true) &&
            !board->square_attacked(7,6,true))
        {
            moves.emplace_back(7,4,7,6); // e1 to g1
        }
        // Queen-side
        rook = board->get_piece(7, 0);
        if (params.WQ_castle &&
            rook && rook->symbol == 'R' && !rook->color &&
            board->get_piece(7,1) == nullptr &&
            board->get_piece(7,2) == nullptr &&
            board->get_piece(7,3) == nullptr &&
            !board->in_check(false) &&
            !board->square_attacked(7,2,true) &&
            !board->square_attacked(7,3,true))
        {
            moves.emplace_back(7,4,7,2); // e1 to c1
        }
    }

    else{ // Black king
        // King-side
        rook = board->get_piece(0, 7);
        if (params.BK_castle &&
            rook && rook->symbol == 'R' && rook->color &&
            board->get_piece(0,5) == nullptr &&
            board->get_piece(0,6) == nullptr &&
            !board->in_check(true) &&
            !board->square_attacked(0,5,false) &&
            !board->square_attacked(0,6,false))
        {
            moves.emplace_back(0,4,0,6); // e8 to g8
        }
        // Queen-side
        rook = board->get_piece(0, 0);
        if (params.BQ_castle &&
            rook && rook->symbol == 'R' && rook->color &&
            board->get_piece(0,1) == nullptr &&
            board->get_piece(0,2) == nullptr &&
            board->get_piece(0,3) == nullptr &&
            !board->in_check(true) &&
            !board->square_attacked(0,2,false) &&
            !board->square_attacked(0,3,false))
        {
            moves.emplace_back(0,4,0,2); // e8 to c8
        }
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
    // get latest board params
	params_t params = board->param_stack.back();

	std::vector<move_t> moves;
    int dir, initial_rank; // dir means dir_rank
    if (this->color == false) {   // White => rank up
        dir = -1;                 
        initial_rank = 6;
    } else {                      // Black => rank down
        dir = 1;                  
        initial_rank = 1;
    }

    int promotion_rank = this->color ? 7 : 0;
    std::array<char,4> promos = { 'q', 'r', 'b', 'k' };

    // Forward move (1 step or can be 2 steps if started at initial_rank)
    int new_rank = rank + dir;
    // Don't go outside the board
    if (new_rank >= 0 && new_rank <= 7){
        piece_t* destination = board->get_piece(new_rank, file);

        // Destination is empty => possible move 
        if (destination == nullptr){
            if (new_rank == promotion_rank){
                for (char promo : promos) {
                    moves.emplace_back(rank, file, new_rank, file, promo);
                }
            }
            else {
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
            {
                if (new_rank == promotion_rank){
                    for (char promo : promos) {
                        moves.emplace_back(rank, file, new_rank, new_file, promo);
                    }
                }
                else {
                    moves.emplace_back(rank, file, new_rank, new_file);
                }
            }
        }
    }

    // En passant capture
    int ep_r = params.ep_rank;
    int ep_f = params.ep_file;
    if (ep_r != -1) {
        int target_rank = rank + dir;
        for (int df : {-1, 1}) {
            int target_file = file + df;
            if (target_rank == ep_r && target_file == ep_f) {
                piece_t* adj = board->get_piece(rank, target_file);
                if (board->get_piece(target_rank, target_file) == nullptr &&
                    adj && adj->symbol == 'P' && adj->color != this->color) {
                    moves.emplace_back(rank, file, target_rank, target_file);
                }
            }
        }
    }

    return moves;
}
