#include <cmath>
#include "board.h"
#include "piece.h"
#include "engine.h"

board_t::board_t() {
    for (int rank = 0; rank < 8; rank++)
        for (int file = 0; file < 8; file++)
            board[rank][file] = nullptr;

    WK_castle = true;
	WQ_castle = true;
	BK_castle = true;
	BQ_castle = true;
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

    WK_castle = true;
	WQ_castle = true;
	BK_castle = true;
	BQ_castle = true;
}

board_t::board_t(std::vector <move_t> move_hist, std::array <piece_t*, 12> pieces) : board_t(pieces) {
    WK_castle = true;
	WQ_castle = true;
	BK_castle = true;
	BQ_castle = true;
    
    for (auto& move: move_hist){
        int fr = move.from_rank, ff = move.from_file;
        int tr = move.to_rank, tf = move.to_file;

        board[tr][tf] = board[fr][ff];
        board[fr][ff] = nullptr; 

        piece_t* moved_piece = board[tr][tf];

        bool is_castle = false;
        int rook_from_f = -1, rook_to_f = -1;

        if (moved_piece &&  moved_piece->symbol == 'K' && std::abs(tf - ff) == 2) {
            if (!moved_piece->color && fr == 7) { // white king on home rank
                if (tf > ff && WK_castle) { rook_from_f = 7; rook_to_f = 5; is_castle = true; } // king-side
                if (tf < ff && WQ_castle) { rook_from_f = 0; rook_to_f = 3; is_castle = true; } // queen-side
            } else if (moved_piece->color && fr == 0) { // black king on home rank
                if (tf > ff && BK_castle) { rook_from_f = 7; rook_to_f = 5; is_castle = true; }
                if (tf < ff && BQ_castle) { rook_from_f = 0; rook_to_f = 3; is_castle = true; }
            }
        }

        if (is_castle) {
            board[fr][rook_to_f] = board[fr][rook_from_f];
            board[fr][rook_from_f] = nullptr;
        }

        // Update castling rights if king or rook moved
        if (moved_piece->symbol == 'K'){
            if (moved_piece->color == false){ // White king
                WK_castle = false;
                WQ_castle = false;
            }
            else{ // Black king
                BK_castle = false;
                BQ_castle = false;
            }
        }
        else if (moved_piece->symbol == 'R'){
            if (moved_piece->color == false){ // White rook
                if (fr == 7 && ff == 0) // a1 rook
                    WQ_castle = false;
                else if (fr == 7 && ff == 7) // h1 rook
                    WK_castle = false;
            }
            else{ // Black rook
                if (fr == 0 && ff == 0) // a8 rook
                    BQ_castle = false;
                else if (fr == 0 && ff == 7) // h8 rook
                    BK_castle = false;
            }
        }

        if(board[tr][tf]->color == false){// White 
            switch(move.promotion)
            {
                case 'q': board[tr][tf] = pieces[QUEEN_W]; break;
                case 'r': board[tr][tf] = pieces[ROOK_W]; break;
                case 'k': board[tr][tf] = pieces[KNIGHT_W]; break;
                case 'b': board[tr][tf] = pieces[BISHOP_W]; break; 
                default: break; 
            }
        } 
        else{ // Black
            switch(move.promotion)
            {
                case 'q': board[tr][tf] = pieces[QUEEN_B]; break;
                case 'r': board[tr][tf] = pieces[ROOK_B]; break;
                case 'k': board[tr][tf] = pieces[KNIGHT_B]; break;
                case 'b': board[tr][tf] = pieces[BISHOP_B]; break; 
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
                int dir = o->color ? 1 : -1; // black attacks down, white up
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

// Works as long as we are guaranteed that the move is a pseudo-legal move from get_available_moves
bool board_t::check_move(move_t* move){
    piece_t* piece = get_piece(move->from_rank, move->from_file);
    piece_t* destination = get_piece(move->to_rank, move->to_file);

    int fr = move->from_rank, ff = move->from_file;
    int tr = move->to_rank, tf = move->to_file;

    if (!piece)
        return false; // No piece at source

    bool is_castle = piece && piece->symbol == 'K' && std::abs(tf - ff) == 2;
    piece_t* rook = nullptr;
    int rook_from_f = -1, rook_to_f = -1;
    if (is_castle) {
        if (!piece->color && fr == 7) {           // white
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; } // king-side
            else         { rook_from_f = 0; rook_to_f = 3; } // queen-side
        } else if (piece->color && fr == 0) {     // black
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; }
            else         { rook_from_f = 0; rook_to_f = 3; }
        }
        if (rook_from_f != -1) rook = board[fr][rook_from_f];
    }

    // simulate
    piece_t* captured = destination;

    // Make the move
    board[tr][tf] = piece;
    board[fr][ff] = nullptr;

    if (is_castle && rook) {
        board[fr][rook_to_f] = rook;
        board[fr][rook_from_f] = nullptr;
    }

    bool self_in_check = in_check(piece->color);

    // Undo the move
    board[fr][ff] = piece;
    board[tr][tf] = captured;

    if (is_castle && rook) {
        board[fr][rook_from_f] = rook;
        board[fr][rook_to_f] = nullptr;
    }

    return !self_in_check;
}

