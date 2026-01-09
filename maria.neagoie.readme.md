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