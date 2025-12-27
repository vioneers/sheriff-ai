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

## Winter Holiday
- On main branch:   
    - Changed penalty sign issue in `evaluate_pawn_structure()`
- On bitboards branch: 
    - Switched to bitboards representation for the file `evaluationbar.cpp` (updated `evaluate_material_and_position()`, `evaluate_mobility`,`evaluate_pawn_structure()`) 
    - Fixes in move generation (especially castling) and perft - perft now outputting right number of total moves, captures, en passants, castlings for given initial board from fen string