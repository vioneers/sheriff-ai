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
- Set up lichess bot to play using our engine by splitting the entrypoints and adding a separate build target `sheriff_ai_lichess` for the bot.
- Set up UCI control and parsing so lichess bot can use the engine via the sheriff_ai_lichess target.

## Week 3
- `move_order_score()` - Computes the priority score of a move, helps in ordering legal moves before looping through them in Alpha-Beta.
- Modified Alpha-beta min/max to modify killer_moves and history tables whenever beta threshold is surpassed. Both tables are heurisitics that again help in ordering legal moves before the loop.
- Computed bot elo using lichess.

## Week 4
- Transition to bitboards :
- Implemented move generation for pawns, knights and king using bitboard
- Implemented `square_attacked()` to check if a square is under attack by a any piece of a given colour.
- Implemented `in_check()` to check if a given colour is under check.
- Optimised three-fold repition tracking and checking using dictionaries and irreversible move tracking.

## Week 5
- Implemented `get_legal_moves()` and `add_move` for the bitboard implementation.
- Implemented `make_move()` and `undo_move()`.
- Resolved ties between moves with same move score heuristic by picking the first move.
- Implemented `move_order_score()` for the bitboard implementation to sort moves before calling the AlphaBeta.
- Added a debugging mode which tracks the three best moves at each position and outputs the lines which gave those bestmoves.
- Modified Alpha-Beta so it scores a faster checkmate as a better sequence of moves.

## Week 6
- Implemented Late move reductions(LMR) which reduce the search depth for later, "worse", moves. If a reduced-depth move seems
to perform well then we perform a full depth search on it.
- Added null-move pruning, this allows us to 'pass' a move so that we can detect positions that are so strong that they remain
good even without making a move, if it is so then we prune the branch, allowing us to reduce the search tree.
- Modified `eval_mobility()` to use bitboards instead of `get_legal_moves()` to count the number of legal moves available for each piece. This is way quicker and helped us increase the depth from 6 to 7 in most cases.
