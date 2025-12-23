#include <cmath>
#include <random>
#include <unordered_map>
#include <cctype>
#include "board.h"
#include "piece.h"
#include "engine.h"

uint64_t random64(uint64_t &x){ // to generate random numbers
    static std::mt19937_64 rng(0xC0FFEE123456789ULL); 
    return rng();
}

int base_index(char ch){
    switch(ch){
        case 'P': return 0;
        case 'N': return 1;
        case 'B': return 2; 
        case 'R': return 3;
        case 'Q': return 4;
        case 'K': return 5;
        default: return -1;
    }
}
int index(piece_t* piece){
    int base = base_index(piece->symbol);
    if (base == -1)
        return -1; 
    return (piece->color ? 6 : 0) + base;
}

uint64_t Z_PSQ[12][64];
uint64_t Z_TURN; 
uint64_t Z_CASTLE[16];
uint64_t Z_EPFILE[9];
bool Z_INIT = false;

void init_zobrist(){
    if (Z_INIT)
        return; 
    uint64_t seed = 0xC0FFEE123456789ULL;

    for (int piece = 0; piece < 12; piece++) // 12 types of pieces
    {
        for (int square = 0; square < 64; square++) // 64 squares on the board
            Z_PSQ[piece][square] = random64(seed);
    }
    // Turn (black or white)
    Z_TURN = random64(seed);

    // Castles
    for (int i = 0; i < 16; i++)
        Z_CASTLE[i] = random64(seed);
    
    // En passant (one extra file for the no en passant case) 
    for (int i = 0; i < 9; i++)
        Z_EPFILE[i] = random64(seed);
}

board_t::board_t() { // init an empty board
    turn = false; // Starting => White

	for (int rank = 0; rank < 8; rank++)
        for (int file = 0; file < 8; file++)
            board[rank][file] = nullptr;

    bool WK_castle = true;
	bool WQ_castle = true;
	bool BK_castle = true;
	bool BQ_castle = true;

    int ep_rank = -1;
    int ep_file = -1;

	bool ep_played = false;

	piece_t* captured = nullptr;

    params_t params {WK_castle, WQ_castle, BK_castle, BQ_castle, ep_rank, ep_file, ep_played, captured};

	param_stack.push_back(params);

    key = compute_key();
    key_history.clear();
    key_history.push_back(key);
    repetition_count.clear();
    repetition_count[key] = 1;
    irreversible_stack.clear();
    last_irreversible_index = 0;
}

board_t::board_t(std::array <piece_t*, 12> pieces): board_t(){ // init board with inital piece placement
	this->pieces = pieces;

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

    key = compute_key();
    key_history.clear();
    key_history.push_back(key);
    repetition_count.clear();
    repetition_count[key] = 1;
    irreversible_stack.clear();
    last_irreversible_index = 0;
}

board_t::board_t(std::vector <move_t> move_hist, std::array <piece_t*, 12> pieces) : board_t(pieces) {
    for (auto& move: move_hist)
		make_move(move);
}

piece_t* board_t::get_piece(int rank, int file) const
{
    return board[rank][file];
}

bool board_t::is_threefold() const{
    auto it = repetition_count.find(key);
    return it != repetition_count.end() && it->second >= 3;
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
bool board_t::check_move(const move_t move){

	piece_t* piece = get_piece(move.from_rank, move.from_file);

	make_move(move);

    bool self_in_check = in_check(piece->color);

	undo_move(move);

    return !self_in_check;
}

std::vector <move_t> board_t::get_legal_moves(){
    std::vector <move_t> legal;
    for (int rank = 0; rank < 8; rank++){
		for (int file = 0; file < 8; file++){
			piece_t* p = get_piece(rank,file); 
			if (p!= nullptr && p->color == turn){ // If the current piece if the color of the current turn
				// Start with pseudo_legal moves returned by get_available_moves (method of piece_t)
				std::vector <move_t> pseudo_legal = p->get_available_moves(this, rank, file); 
				for(auto& move : pseudo_legal){ // Check all pseudo_legal moves in the current configuration of the board (method of board_t)
					if(check_move(move))
						legal.push_back(move);
				}
			}
		}
	}
    return legal;
}

uint64_t board_t::compute_key() const{
    init_zobrist();
    uint64_t key = 0;
    for (int r = 0; r < 8; r++){
        for (int f = 0; f < 8; f++){
            piece_t* piece = board[r][f]; 
            if (piece == nullptr)
                continue; 
            key ^= Z_PSQ[index(piece)][r*8 + f]; 
        }
    }

    if (turn) // White starts
        key ^= Z_TURN;
    
    params_t params = param_stack.back();
    int castle_mask = 0; 
    // Set one bit for each type of castle
    if (params.WK_castle) 
        castle_mask |= 1; 
    if (params.WQ_castle)
        castle_mask |= 2; 
    
    if (params.BK_castle)
        castle_mask |= 4;
    if (params.BQ_castle)
        castle_mask |= 8;
    key ^= Z_CASTLE[castle_mask];

    //En passant
    int ep;
    if (params.ep_file == -1) // no en passant
        ep = 8;
    else
        ep = params.ep_file; // possible files 0 - 7
    key ^= Z_EPFILE[ep];
    return key;
}

// Assuming verified legal move
void board_t::make_move(const move_t move){
    int tr = move.to_rank; 
    int tf = move.to_file;
    int fr = move.from_rank;
    int ff = move.from_file;

    uint64_t old_key = key;

    params_t prev_params = param_stack.back();
	// initialise new params with previous values
	params_t params = prev_params;

	// reset necessary params
	params.ep_rank = -1;
	params.ep_file = -1;
	params.ep_played = false;

	// move piece and capture
	piece_t* piece = board[fr][ff];
	params.captured = board[tr][tf];

    // Make the move
    board[tr][tf] = board[fr][ff]; 
    board[fr][ff] = nullptr;

	// Record last moved pawn eligible for en passant
	if (piece->symbol == 'P' && std::abs(tr - fr) == 2) {
		params.ep_rank = fr + (piece->color ? 1 : -1);  // square jumped over
		params.ep_file = ff;
	}	

	// Handle en passant capture, as we cannot know if the move is en passent or not without the current board configuration
	if (piece->symbol == 'P' && params.captured == nullptr &&
	std::abs(tf - ff) == 1 && (tr - fr == (piece->color ? 1 : -1))) {
		int cap_r = fr;      // same rank as pawn started
		int cap_f = tf;      // file it moved into
		piece_t* ep_pawn = board[cap_r][cap_f];
		if (ep_pawn && ep_pawn->symbol == 'P' && ep_pawn->color != piece->color) {
			params.ep_played = true;
			params.captured = ep_pawn;
			board[cap_r][cap_f] = nullptr; // remove the captured pawn
		}
	}

    // Promotion 
    if (move.promotion){
        if (turn == 0){ // White
            switch(move.promotion){
            case 'q': board[tr][tf] = pieces[QUEEN_W]; break;
            case 'r': board[tr][tf] = pieces[ROOK_W]; break;
            case 'k': board[tr][tf] = pieces[KNIGHT_W]; break;
            case 'b': board[tr][tf] = pieces[BISHOP_W]; break;
            }
        }
        else{ // Black
            switch(move.promotion){
            case 'q': board[tr][tf] = pieces[QUEEN_B]; break;
            case 'r': board[tr][tf] = pieces[ROOK_B]; break;
            case 'k': board[tr][tf] = pieces[KNIGHT_B]; break;
            case 'b': board[tr][tf] = pieces[BISHOP_B]; break;
            }
        }
    }
    else if (piece->symbol == 'K' && std::abs(tf - ff) == 2) { // Move the rook if castling
        int rook_from_f = -1, rook_to_f = -1;

		if (!piece->color && fr == 7) {           // white
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; } // king-side
            else         { rook_from_f = 0; rook_to_f = 3; } // queen-side
        } else if (piece->color && fr == 0) {     // black
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; }
            else         { rook_from_f = 0; rook_to_f = 3; }
        }

        board[fr][rook_to_f] = board[fr][rook_from_f];
        board[fr][rook_from_f] = nullptr;
    }
	
	// if king has moved we can no longer castle
    if (piece->symbol == 'K')
        if (!piece->color)
            params.WK_castle = params.WQ_castle = false;
        else
            params.BK_castle = params.BQ_castle = false;
	
	// if rook has moved we can no longer castle
    if (piece->symbol == 'R')
        if (!piece->color)
            if (tf > ff) params.WK_castle = false;
            else params.WQ_castle = false;
        else 
            if (tf > ff) params.BK_castle = false;
            else params.BQ_castle = false;

	// update the turn
	turn = 1 - turn;

	// update params
	param_stack.push_back(params);

    auto castle_mask_from = [](const params_t& p){
        int mask = 0;
        if (p.WK_castle) mask |= 1;
        if (p.WQ_castle) mask |= 2;
        if (p.BK_castle) mask |= 4;
        if (p.BQ_castle) mask |= 8;
        return mask;
    };

    uint64_t new_key = old_key;
    // toggle turn
    new_key ^= Z_TURN;

    // castle rights
    int old_castle_mask = castle_mask_from(prev_params);
    int new_castle_mask = castle_mask_from(params);
    if (old_castle_mask != new_castle_mask){
        new_key ^= Z_CASTLE[old_castle_mask];
        new_key ^= Z_CASTLE[new_castle_mask];
    }

    // en passant
    int old_ep = prev_params.ep_file == -1 ? 8 : prev_params.ep_file;
    int new_ep = params.ep_file == -1 ? 8 : params.ep_file;
    if (old_ep != new_ep){
        new_key ^= Z_EPFILE[old_ep];
        new_key ^= Z_EPFILE[new_ep];
    }

    int from_sq = fr * 8 + ff;
    int to_sq = tr * 8 + tf;

    // remove moving piece from origin
    new_key ^= Z_PSQ[index(piece)][from_sq];

    // remove captured piece
    if (params.captured){
        int cap_r = params.ep_played ? fr : tr;
        int cap_f = tf;
        new_key ^= Z_PSQ[index(params.captured)][cap_r * 8 + cap_f];
    }

    // add moving piece at destination (handle promotion)
    if (move.promotion){
        char promo_sym = std::toupper(move.promotion);
        if (promo_sym == 'K') promo_sym = 'N'; // 'k' is used for knight promotion
        int promo_idx = (piece->color ? 6 : 0) + base_index(promo_sym);
        new_key ^= Z_PSQ[promo_idx][to_sq];
    } else {
        new_key ^= Z_PSQ[index(piece)][to_sq];
    }

    // handle rook movement during castling
    if (piece->symbol == 'K' && std::abs(tf - ff) == 2){
        int rook_from_f = (tf > ff) ? 7 : 0;
        int rook_to_f   = (tf > ff) ? 5 : 3;
        piece_t* rook_piece = piece->color ? pieces[ROOK_B] : pieces[ROOK_W];
        new_key ^= Z_PSQ[index(rook_piece)][fr * 8 + rook_from_f];
        new_key ^= Z_PSQ[index(rook_piece)][fr * 8 + rook_to_f];
    }

    key = new_key;
    key_history.push_back(key);

    // Track repetition counts efficiently
    irreversible_stack.push_back(last_irreversible_index);
    bool castle_changed = (prev_params.WK_castle != params.WK_castle) ||
                          (prev_params.WQ_castle != params.WQ_castle) ||
                          (prev_params.BK_castle != params.BK_castle) ||
                          (prev_params.BQ_castle != params.BQ_castle);
    bool irreversible = params.captured != nullptr ||
                        piece->symbol == 'P' ||
                        move.promotion ||
                        castle_changed;

    if (irreversible){
        last_irreversible_index = static_cast<int>(key_history.size()) - 1;
        repetition_count.clear();
        repetition_count[key] = 1;
    } else {
        repetition_count[key] += 1;
    }
}

void board_t::undo_move(const move_t move){
    int tr = move.to_rank; 
    int tf = move.to_file;
    int fr = move.from_rank;
    int ff = move.from_file;

	// pop parameters from the last board state
	params_t params = param_stack.back();
	param_stack.pop_back();

    piece_t* piece = get_piece(tr, tf);

	// Undo the move
    board[fr][ff] = piece;
    board[tr][tf] = nullptr;

	// put back captured pieces
	if(!params.ep_played)
		board[tr][tf] = params.captured;
	else
		board[fr][tf] = params.captured;

	// if undoing castling, move the rook back
	if(piece->symbol == 'K' && std::abs(tf - ff) == 2)
	{
		int rook_from_f = -1, rook_to_f = -1;

        if (!piece->color && fr == 7) {           // white
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; } // king-side
            else         { rook_from_f = 0; rook_to_f = 3; } // queen-side
        } else if (piece->color && fr == 0) {     // black
            if (tf > ff) { rook_from_f = 7; rook_to_f = 5; }
            else         { rook_from_f = 0; rook_to_f = 3; }
        }

		// move back the rook
        board[fr][rook_from_f] = board[fr][rook_to_f];
        board[fr][rook_to_f] = nullptr;
	}

    if (move.promotion){
        // revert promoted piece back to pawn
        board[fr][ff] = piece->color ? pieces[PAWN_B] : pieces[PAWN_W];
    }

	// undo the turn
	turn = 1 - turn;

    // Update repetition tracking
    int prev_last_irreversible = irreversible_stack.back();
    irreversible_stack.pop_back();
    bool was_irreversible = prev_last_irreversible != last_irreversible_index;
    uint64_t current_key = key_history.empty() ? 0 : key_history.back();

    if (!was_irreversible){
        auto it = repetition_count.find(current_key);
        if (it != repetition_count.end()){
            if (--(it->second) == 0)
                repetition_count.erase(it);
        }
    }

    if (!key_history.empty())
        key_history.pop_back();

    if (!key_history.empty())
        key = key_history.back(); 
    else
        key = compute_key();

    last_irreversible_index = prev_last_irreversible;
    if (was_irreversible){
        repetition_count.clear();
        for (int i = last_irreversible_index; i < static_cast<int>(key_history.size()); ++i)
            repetition_count[key_history[i]] += 1;
    }
}
