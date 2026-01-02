# sheriff-ai Bitboards

This is a short outline of the purpose of each new file to make development a bit easier.

## `types.h`
- Definitions of the `Bitboard` type and any constants / enums.

## `bitboard.h`
- Mainly definitions of precomputed masks such as for the piece movements.
- `constexpr` allows the masks to be computed once at compile time so we dont have to recalculate them every time we play.
- Here we can also add any helper fucntions for dealing with `Bitboards`

## `move.h` / `move.cpp`
- The move class now only contains a single integer which encodes the to and from positions as well as some flags (see `types.h` for possible flags)
- I am not sure how useful flags will be in general since we cant always set them from just the UCI code. Maybe we can set them in `make_move()` and use them in `undo_move()`.

## `board.h` / `board.cpp`
- `state_t.h` should contain any flags that need to be retored by `undo_move()`
- Functions that generate moves such as `get_legal_moves`, `add_move`, `generate_pawn_moves`, etc should append their moves to the `list` parameter. This is to avoid copying big lists.
- Functions to generate moves for each piece type are implemented in `pieces.cpp` (just to control the siez of `board.cpp`)
- The porcess of generating legal moves could go something like:
    - call `get_legal_moves(list, color)`
    - for each piece of a certain color call the adequate `generate_..._moves<color>(pseudolegal_list)`
    - (I now realize we actually need to copy from `pseudolegal_list` to `list`. May need to be rethough) `add_move` checks every pseudolegal move and adds only the legal moves to `list`.

## `piece.cpp`
- Implementations of fuctions to generate pseudolegal moves.
- We use templates since this prevents us from always checking which player is moving.

## `debug.h`
- Definitions of `ASSERT` that we can use to test our code in debug mode.
- to switch to debug mode run and then compile:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
- to go back to release mode run and then compile:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
