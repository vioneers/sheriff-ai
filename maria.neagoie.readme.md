# Weekly progress for Maria Neagoie

## Week 1
- Created GitLab repository, set up files structure 
- Configured the project to build using CMake
- Implemented `piece.h` and `piece.cpp` (structures for each piece type and corresponding `get_available_moves` function, returning a std vector of all pseudo-legal moves, disregarding check, checkmate etc.) 

## Week 2
- Implemented `engine.h` and `engine.cpp`
- Additions to `board.h` and `board.cpp` for initialization of board based on moves history
- Combined all components in `main.cpp`, outputing the first entry of the legal moves vector
- Minor fixes in `piece_t` and `move_t`
- Implemented AlphaBeta algorithm

## Week 3
- Implemented Perft algorithm to check the number of generated legal moves is accurate
- Added timing to ensure no time-out. If close to time-out, output random legal move.
- Analyzed Git stats to identify edge cases, noting only three draws under the threefold repetition rule. Worked on threefold prevention, not yet completed.

## Week 4 
- Threefold repetition handling with Zobrist hashing
    - Implemented `init_zobrist()`,`compute_key()`, `is_threefold()` and updated the key history in `make_move()` and `undo_move()` 
    - `alphaBetaMax` and `alphaBetaMin` evaluate threefold repetition with a score of 0, avoiding it when a better move exists and performing it only if the best outcome is the draw
### Still for Week 4 but in Winter Break
- Initial representation: Changed penalty sign issue in `evaluate_pawn_structure()`
- Bitboards representation: 
    - Switched to bitboards representation for the file `evaluationbar.cpp` (updated `evaluate_material_and_position()`, `evaluate_mobility`,`evaluate_pawn_structure()`) 
    - Fixes in move generation (especially castling) and perft - perft now outputting right number of total moves, captures, en passants, castlings for given initial board from fen string
    - Threefold repetition handling with Zobrist hashing for bitboards representation (`z_key` updated incrementally in `make_move()` to be more efficient than calling `compute_key()` every time)
- Merged main (initial board representation) and bitboards branches, ensuring full commit history is preserved. The main branch now contains bitboards implementation. 

## Week 5
- Implemented `opening_book.h` and `opening_book.cpp` - selected a few openings ranked as best in chess (vector `OPENINGS`), implemented functions `init_lookup_table()`, `legal_move_uci()`, `probe()` (but not yet used by our engine - to be done later)
- Analysed game logs of this Thursday - we draw on both threefold and fifty move rule
- Implemented Quiesence Search - functions `quiesenceSearchMax()` and `quiesenceSearchMin()` -> called in `alphaBetaMax()` and `alphaBetaMin()` when `depth_left = 0` to extend the search on capture and promotion moves, and reduce horizon effect

## Week 6
- Implemented `is_repetition(int cnt_rep)`: checks if the current board has occured cnt_rep times already; used to check threefold and to penalize repetitions (even if only 2) in `evaluate()`
- Added Iterative Deepening - `get_best_move()`: wrapper for AlphaBeta
- Fixes done to: 
    - `eval_king_confinement()` to consider the white king as well in the scoring; 
    - `evaluate()` to use absolute value because mate is +/-(MATE_SCORE - ply); 
    - `eval_passed_pawns()` to consider direction "in front" according to whether the pawn is black or white;
    - `eval_mobility()` to return the difference mobility for white - mobility for black instead of just adding a sign for side to move
- Added a `plies_since_irrev()` method in board -> used to detect 50-move draws in AlphaBeta and Quiesence Search + updated `score_moves()` and `move_order_score()` to give bonus to the score of a capture / pawn move if we are approaching a 50-move and we are in a winning position
- Added prints to console for the logs of games in `random_bot.cpp` (play 50 games as White and 50 as Black) to check victory rate against random bot 
- Linked the openings to our engine in `engine.get_strategy()` -> If the current position hash is in the openings, play the known best move at that point. Otherwise, do the search with `engine.get_best_move()`. (This was commented out for Week 6 because we had time-outs when running stats during TD.)

## Week 7
- Changes to the way we link openings to our engine (especially to avoid timeouts we had):
    - Realised legality check from `init_lookup_table()` cannot be removed because of flags the uci might not show, so even though it's costly, we have to keep `legal_move_uci()`
    - Reserved an upperbound on size of table to avoid rehashes
    - Added `OpeningBook& get_openings()` and an `initialized` parameter (used when debugging to ensure we only call `init_lookup_table()` once) 
    - Moved `init_zobrist()` to main