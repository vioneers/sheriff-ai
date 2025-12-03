# Weekly changelog
## Week 1
Additions :
- `check_move()` - To check if a move is legal in the current state of the board.
- `in_check()` - Checks if the king of a given color is under check or not.
- `square_attacked()` - Checks if a given square on the board is attacked by any piece of a certian color.
- `check_in_between()` - Given two squares on the board, checks if any piece obstructs the path between the two squares.

## Week 2
- Added promotion moves to `get_available_moves()`.
- Removed legal move checks duplicated between `get_available_moves()` and `check_move`.
- Implemented castling and en passant generation and validation/replay.

