# Weekly changelog
## Week 1
- `move_t` to store moves and convert to/from UCI encoding
- argument parsing in `main.cpp`
- reading and writing to/from files

## Week 2
- wrote `make_move` and `undo_move` functions

## Week3
- started work on bitboards backend on separate branch
- implemented magic number generation
- added debugging functionallity

## Week 4
- implemented move generation for rook, bishop and queen
- added castling move generation
- added `board_t` constructor from fen as well as to_fen function for debugging
- multiple minor changes and big fixes

## Week 5
- implemented transposition tables
- minor optimizations for move ordering
- - added testing agains random bot
- integrated transposition tables with null move pruning

## Week 6
- **MUCH MUCH** debugging
- optimized static eval to use bitboards