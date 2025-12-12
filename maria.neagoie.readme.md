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