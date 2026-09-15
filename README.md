# Sheriff-AI

A UCI-compatible chess engine written from scratch in C++20. It uses bitboard-based move generation with magic bitboards for sliding pieces, and searches with alpha-beta pruning with many aditional standard optimisations (transposition tables, iterative deepening, quiescence search, null-move pruning, late move reductions, and more).

The project was developed as the final team assignment for CSC_2F001_EP Object-Oriented Programming in C++ at École Polytechnique.

Sheriff-AI has been optimised for speed, so as to fit within the 10s per move allowed by the course evalutaion. It has achived an elo rating of over [1800](https://lichess.org/@/SheriffAI_Bot) by playing hundreds of games against other bots on [Lichess](https://lichess.org).

## Features

- **Bitboard board representation** — the board is stored as a set of 64-bit integers (one per piece type, plus occupancy boards), with magic bitboards used to generate sliding-piece (bishop/rook/queen) attacks in O(1).
- **Full legal move generation** — pawns (incl. double pushes, en passant, promotions), knights, kings (incl. castling), and sliding pieces, validated against a perft (move-path enumeration) test suite.
- **Alpha-beta search** with:
  - Iterative deepening with aspiration windows
  - Principal variation search (PVS)
  - Quiescence search with delta pruning (to avoid the horizon effect on captures/promotions)
  - Null-move pruning
  - Late move reductions (LMR)
  - Check extensions
  - Move ordering via killer moves, history heuristic (with aging), and MVV-style scoring
- **Transposition table** keyed by incrementally-updated Zobrist hashes, storing bound type, depth, and best move.
- **Threefold-repetition and fifty-move-rule detection**, incorporated directly into search scoring so the engine avoids repetition when it's winning.
- **Tapered evaluation** (piece-square tables blended between middlegame/endgame by game phase), with terms for material, mobility, pawn structure, passed pawns, king safety/confinement, rook activity, and bishop pair.
- **Opening book** of hand-picked mainline openings, probed by Zobrist key before search kicks in.
- **Two front ends**: a simple file-based UCI-move interface (used by the class's engine-vs-engine tournament harness) and a full UCI loop for running the engine as a **Lichess bot**.
- **Debug tooling**: assertion macros compiled out in release builds, and principal-variation logging for inspecting search behaviour.

## Getting Started

### Prerequisites
- A C++20 compiler (MSVC, Clang, or GCC)
- [CMake](https://cmake.org/) 3.15+

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

For debug builds (enables `ASSERT`/`ASSERT_BOARD` checks):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

The first build automatically compiles and runs `make_magic`, which generates `src/magic.h` (the magic-bitboard lookup constants). This file is gitignored and regenerated on demand — delete it to force regeneration.

### Executables

| Target | Description |
|---|---|
| `sheriff_ai` | Reads a move history from a file and writes the engine's chosen move to another file (`-H <input> -m <output>`). This is the interface used by the class's tournament/grading harness. |
| `sheriff_ai_lichess` | Speaks the [UCI protocol](https://en.wikipedia.org/wiki/Universal_Chess_Interface) over stdin/stdout, so it can be plugged into a Lichess bot bridge (e.g. [lichess-bot](https://github.com/lichess-bot-devs/lichess-bot)) or any UCI-compatible GUI. |
| `sheriff_ai_perft` | Runs a [perft](https://www.chessprogramming.org/Perft) test from a fixed FEN, printing node counts and capture/castle/en-passant/promotion breakdowns per depth, to verify move generation correctness. |
| `random_game` | Plays the engine against a random-move bot across many multithreaded games, logging each game to `game_logs/` — used to measure win rate and catch rules bugs (repetition, fifty-move, stalemate, etc.). |
| `make_magic` | Standalone generator for the magic bitboard constants used by sliding-piece move generation; run automatically as part of the build. |

## Project Structure

### Core types
- **[`types.h`](src/types.h)** — Fundamental types shared across the engine: the `Bitboard` alias, `PieceType`/`Color`/`Square` enums, move-flag constants (capture, castle, en passant, promotions), and transposition-table node types.
- **[`bitboard.h`](src/bitboard.h)** — Compile-time-generated bitboard constants and helpers: rank/file masks, knight/king attack tables, and the magic-bitboard machinery (masks, blocker enumeration, and the attack lookup tables for rooks/bishops built from `magic.h`).
- **[`magic.h`](src/magic.h)** *(generated, not tracked in git)* — The actual magic numbers and shift constants for each square, produced by `make_magic.cpp`.

### Board and moves
- **[`board.h`](src/board.h) / [`board.cpp`](src/board.cpp)** — The `board_t` struct: bitboard piece sets, a mailbox array for O(1) piece lookup, and the full game-state stack (`state_t`) needed to make/undo moves. Implements `make_move`/`undo_move`, null-move helpers (for null-move pruning), check/attack detection (`in_check`, `square_attacked`), stalemate detection, FEN parsing/serialization, and repetition/fifty-move tracking via Zobrist keys.
- **[`movegen.h`](src/movegen.h)** — Template implementations of `board_t`'s per-piece pseudo-legal move generators (pawns, knights, king incl. castling, and magic-bitboard-driven sliding pieces), templated on `Color` to avoid runtime branching on side-to-move. Included at the bottom of `board.h`.
- **[`move.h`](src/move.h) / [`move.cpp`](src/move.cpp)** — The `move_t` struct: a move is packed into a single 16-bit integer (from-square, to-square, flags) plus a separate ordering score used during search. Includes conversion to/from UCI long algebraic notation (e.g. `e2e4`, `e7e8q`).

### Search and evaluation
- **[`engine.h`](src/engine.h) / [`engine.cpp`](src/engine.cpp)** — The `engine_t` struct and the search itself: alpha-beta with iterative deepening, aspiration windows, PVS, null-move pruning, late move reductions, check extensions, quiescence search, killer-move/history-heuristic move ordering, and the transposition-table-backed cutoffs. `get_strategy()` is the entry point — it consults the opening book first, then falls back to `get_best_move()`.
- **[`transposition.cpp`](src/transposition.cpp)** — Transposition table probing (`TTprobe`), storing (`TTstore`), and the cutoff logic (`checkTT`) that turns a TT hit into an early return from search, including bound-type (exact/lower/upper) handling and mate-score normalization.
- **[`evaluationbar.h`](src/evaluationbar.h) / [`evaluationbar.cpp`](src/evaluationbar.cpp)** — Static position evaluation: tapered piece-square tables, material, mobility, pawn structure (doubled/passed/isolated pawns), king safety, rook/bishop bonuses, blended by a `phase()` function that estimates how far the game is into the endgame. Also exposes a small CLI evaluation bar for debugging positions.
- **[`zobrist.h`](src/zobrist.h) / [`zobrist.cpp`](src/zobrist.cpp)** — Zobrist hashing: random keys per (piece, square), side-to-move, castling rights, and en-passant file, combined into a single incrementally-updated 64-bit key used by the transposition table and repetition detection.
- **[`opening_book.h`](src/opening_book.h) / [`opening_book.cpp`](src/opening_book.cpp) / [`openings.h`](src/openings.h)** — A small opening book: `openings.h` holds a hand-picked list of strong opening lines in UCI move sequences; `opening_book.cpp` builds a Zobrist-key → best-move lookup table from them and probes it before the engine falls back to search.

### Entry points
- **[`main.cpp`](src/main.cpp)** — File-based entry point (`sheriff_ai`): Used by the class's engine-vs-engine tournament infrastructure.
- **[`main_lichess.cpp`](src/main_lichess.cpp)** — UCI entry point (`sheriff_ai_lichess`): implements the subset of the UCI protocol needed to run the engine as a Lichess bot.
- **[`perft.cpp`](src/perft.cpp)** — Standalone perft correctness test (`sheriff_ai_perft`) against a fixed FEN, used to validate move generation against known-correct node counts.
- **[`random_game.cpp`](src/random_game.cpp)** — Runs many multithreaded games against a random-move opponent, for measuring engine strength and hunting for rules bugs.
- **[`make_magic.cpp`](src/make_magic.cpp)** — Generates `src/magic.h` by randomized trial search for magic numbers that perfectly hash blocker configurations to attack sets for rooks and bishops on every square.

### Utilities
- **[`utils.h`](src/utils.h)** — Small shared helpers: command-line argument parsing, reading/writing move histories from files, and UCI command tokenizing (used by `main.cpp` and `main_lichess.cpp`).
- **[`debug.h`](src/debug.h)** — `ASSERT`/`ASSERT_BOARD` macros that check invariants (and dump the board's FEN on failure) in debug builds, and compile to nothing in release builds.

### Legacy (superseded, not built)
Earlier in development the engine used an object-oriented, mailbox-only board representation before migrating to the bitboard design above. These files are kept for history but are **not** part of the CMake build:
- **[`board_old.h`](src/board_old.h) / [`board_old.cpp`](src/board_old.cpp)** — the original `board_t`/`params_t` implementation.
- **[`piece.h`](src/piece.h) / [`piece.cpp`](src/piece.cpp)** — the original per-piece-type class hierarchy (`king_t`, `queen_t`, etc.) used to generate moves before templated, bitboard-based generation replaced it.

## Acknowledgments

- Search and evaluation techniques largely follow [chessprogramming.org](https://www.chessprogramming.org).

## Contributors

- Maria Neagoie
- Luxin Matasaru
- Shounak Rao
- Omar Cherif
