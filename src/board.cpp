#include <cmath>
#include "board.h"
#include "piece.h"
#include "engine.h"

board_t::board_t() {
    for (int rank = 0; rank < 8; rank++)
        for (int file = 0; file < 8; file++)
            board[rank][file] = nullptr;
}

board_t::board_t(std::array <piece_t*, 12> pieces){   // Initial board
    for (int i = 0; i < 8 ; i++) {
        for (int j = 0; j < 8; j++)
            board[i][j] = nullptr;
    }

    // Pawns
    for (int j = 0; j < 8; j++) {
        board[1][j] = pieces[PAWN_B];
        board[6][j] = pieces[PAWN_W];
    }

    // Black back rank
    board[0][0] = pieces[ROOK_B];
    board[0][1] = pieces[KNIGHT_B];
    board[0][2] = pieces[BISHOP_B];
    board[0][3] = pieces[QUEEN_B];
    board[0][4] = pieces[KING_B];
    board[0][5] = pieces[BISHOP_B];
    board[0][6] = pieces[KNIGHT_B];
    board[0][7] = pieces[ROOK_B];

    // White back rank
    board[7][0] = pieces[ROOK_W];
    board[7][1] = pieces[KNIGHT_W];
    board[7][2] = pieces[BISHOP_W];
    board[7][3] = pieces[QUEEN_W];
    board[7][4] = pieces[KING_W];
    board[7][5] = pieces[BISHOP_W];
    board[7][6] = pieces[KNIGHT_W];
    board[7][7] = pieces[ROOK_W];
}

board_t::board_t(std::vector <move_t> move_hist, std::array <piece_t*, 12> pieces) : board_t(pieces) {
    for (auto& move: move_hist){
        board[move.to_rank][move.to_file] = board[move.from_rank][move.from_file];
        board[move.from_rank][move.from_file] = nullptr; 
        if(board[move.to_rank][move.to_file]->color == false){// White 
            switch(move.promotion)
            {
                case 'q': board[move.to_rank][move.to_file] = pieces[QUEEN_W]; break;
                case 'r': board[move.to_rank][move.to_file] = pieces[ROOK_W]; break;
                case 'k': board[move.to_rank][move.to_file] = pieces[KNIGHT_W]; break;
                case 'b': board[move.to_rank][move.to_file] = pieces[BISHOP_W]; break; 
                default: break; 
            }
        } 
        else{ // Black
            switch(move.promotion)
            {
                case 'q': board[move.to_rank][move.to_file] = pieces[QUEEN_B]; break;
                case 'r': board[move.to_rank][move.to_file] = pieces[ROOK_B]; break;
                case 'k': board[move.to_rank][move.to_file] = pieces[KNIGHT_B]; break;
                case 'b': board[move.to_rank][move.to_file] = pieces[BISHOP_B]; break; 
                default: break; 
            }
        }
        
    }
}

piece_t* board_t::get_piece(int rank, int file)
{
    return board[rank][file];
}

bool board_t::check_in_between(int fr, int ff, int tr, int tf) const {
    int step_r = (tr > fr) - (tr < fr); 
    int step_f = (tf > ff) - (tf < ff); 
    for (int r = fr + step_r, f = ff + step_f; r != tr || f != tf; r += step_r, f += step_f) {
        if (board[r][f]) return true; // something blocks the way
    }
    return false;
}

bool board_t::square_attacked(int r, int f, bool by_color) const {
    for (int sr = 0; sr < 8; ++sr) for (int sf = 0; sf < 8; ++sf) {
        piece_t* o = board[sr][sf];
        if (!o || o->color != by_color) continue;
        int dr = r - sr, df = f - sf;
        switch (o->symbol) {
            case 'P': {
                int dir = o->color ? -1 : 1; // black attacks down, white up
                if (dr == dir && std::abs(df) == 1) 
                    return true;
                break;
            }
            case 'N':
                if ((std::abs(dr) == 2 && std::abs(df) == 1) || (std::abs(dr) == 1 && std::abs(df) == 2))
                    return true;
                break;
            case 'B':
                if ((dr || df) && std::abs(dr) == std::abs(df) && !check_in_between(sr, sf, r, f))
                    return true;
                break;
            case 'R':
                if ((dr || df) && (dr == 0 || df == 0) && !check_in_between(sr, sf, r, f))
                    return true;
                break;
            case 'Q':
                if ((dr || df) && ((dr == 0 || df == 0) || (std::abs(dr) == std::abs(df))) && !check_in_between(sr, sf, r, f))
                    return true;
                break;
            case 'K':
                if (std::abs(dr) <= 1 && std::abs(df) <= 1 && (dr || df)) 
                    return true;
                break;
        }
    }
    return false;
}

bool board_t::in_check(bool color) const {
    // Find the king
    int kr = -1, kf = -1;
    for (int r = 0; r < 8 && kr == -1; ++r){
        for (int f = 0; f < 8; ++f) {
            piece_t* p = board[r][f];
            if (p && p->color == color && p->symbol == 'K') {
                kr = r; kf = f;
                break;
            }
        }
    }
    if (kr == -1 || kf == -1) return false; // no king found, should not happen
    return square_attacked(kr, kf, !color);
}

bool board_t::check_move(move_t* move){
    // We need to account for our king being under check or any of the pieces being pinned, both being board states.
    piece_t* piece = get_piece(move->from_rank, move->from_file);
    piece_t* destination = get_piece(move->to_rank, move->to_file);

    int fr = move->from_rank, ff = move->from_file;
    int tr = move->to_rank, tf = move->to_file;

    if (piece == nullptr)
        return false; // No piece at source

    if (destination != nullptr && piece->color == destination->color)
        return false; // Can't capture own piece

    if (fr == tr && ff == tf)
        return false; // No movement

    bool legal_shape = false;
    switch (piece->symbol){
        case 'K': // King
        {
            legal_shape = (std::abs(tr - fr) <= 1 && std::abs(tf - ff) <= 1) 
                            && !square_attacked(tr, tf, !piece->color); // Can't move more than 1 square
            
            break;
        }

        case 'Q': // Queen
        {
            if (tr == fr || tf == ff || std::abs(tr - fr) == std::abs(tf - ff)){
                // Here we check for pieces in between
                legal_shape = !check_in_between(fr, ff, tr, tf);
            }
            break;
        }

        case 'P': // Pawn
        {
            int dir = piece->color ? -1 : 1; // Direction of movement
            // Standard move
            if (ff == tf){
                if (tr - fr == dir && destination == nullptr)
                    legal_shape = true; // Move forward 1
                if ((fr == 1 && dir == 1) || (fr == 6 && dir == -1)){
                    if (tr - fr == 2 * dir && destination == nullptr && get_piece(fr + dir, ff) == nullptr)
                        legal_shape = true; // Move forward 2 from initial position
                }
            }
            // Capture move
            else if (std::abs(tf - ff) == 1 && tr - fr == dir){
                if (destination != nullptr && destination->color != piece->color)
                    legal_shape = true; 
            }
            break;
        }
        case 'N': // Knight
        {
            legal_shape = (std::abs(tr - fr) == 2 && std::abs(tf - ff) == 1) || (std::abs(tr - fr) == 1 && std::abs(tf - ff) == 2);
    
            break;
        }
        case 'B': // Bishop
        {
            if (std::abs(tr - fr) == std::abs(tf - ff)){
                // Here we check for pieces in between
                legal_shape = !check_in_between(fr, ff, tr, tf);
            }
            break;
        }
        case 'R': // Rook
        {
            if (tr == fr || tf == ff){
                // Here we check for pieces in between
                legal_shape = !check_in_between(fr, ff, tr, tf);
            }
            break;
        }
        default:
            break;
    }        

    if (!legal_shape)
        return false;

    // simulate
    piece_t* captured = destination;

    // Make the move
    board[tr][tf] = piece;
    board[fr][ff] = nullptr;

    bool self_in_check = in_check(piece->color);

    // Undo the move
    board[fr][ff] = piece;
    board[tr][tf] = captured;

    return !self_in_check;
}


