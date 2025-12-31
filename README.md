<p align="center">
<img src="https://github.com/user-attachments/assets/0c6d308f-def4-4b25-adae-9cb41cf868d5" width="240" alt="Echo logo">
</p>

<h1 align="center"><b>E C H O — A Bitboard Chess Engine</b></h1>
<p align="center"><sub>~2650 Elo · C · UCI · Alpha–Beta · Magic Bitboards</sub></p>

<p align="center">
  <img alt="language" src="https://img.shields.io/badge/language-C-blue">
  <img alt="platform" src="https://img.shields.io/badge/platform-UCI-yellow">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-lightgrey">
  <img alt="status" src="https://img.shields.io/badge/status-active-black">
</p>

# Introduction

Echo is a bitboard chess engine focused on performance and a classical alpha–beta search architecture and evaluation. Chess programming is a rabbit hole that never ends...

# Key Concepts

- **Protocol:** UCI (Universal Chess Interface) compatible.

### Search
- **Algorithm:** Negamax (MiniMax) with Alpha–Beta Pruning.
- **Refinements:** Principal Variation Search (PVS), Iterative Deepening, Aspiration Windows.
- **Pruning:** Null Move Pruning, Futility Pruning, Reverse Futility Pruning (Static Null Move).
- **Heuristics:** Killer Moves, History Heuristic, Late Move Reductions (LMR).
- **Quiescence:** Specialized search to mitigate horizon effects.
- **Optimization:** Transposition Table with Zobrist Hashing.
- **Multi-threading:** Parallel Search using openmp library

### Evaluation
- **Material & Position:** Piece-Square Tables (PST) with middlegame/endgame interpolation.
- **Pawn Structure:** Evaluation of passed, connected, doubled, and isolated pawns.
- **King Safety:** Pawn shield analysis, battery detection, and Chebyshev distance-2 zone infiltration.
- **Piece Activity:** Mobility scores, Knight outposts, Rook open files & 7th rank bonuses.

# Project Structure

The codebase is modularized into the `src/` directory, separating concerns between board representation, search, evaluation, and protocol handling.

```text
/
├── src/
│   ├── main.c           # Entry point
│   ├── board.c          # Board state, bitboard macros, FEN parsing
│   ├── search.c         # Search algorithms (Negamax, PVS, Quiescence)
│   ├── eval.c           # Static evaluation logic & PSTs
│   ├── move_gen.c       # Move generation (pseudo-legal & legal)
│   ├── tt.c             # Transposition table & Zobrist hashing
│   ├── uci.c            # UCI protocol handling & input loop
│   ├── magic.c          # Magic bitboard generation (Rook/Bishop sliders)
│   ├── masks.c          # Pre-calculated attack masks (Leapers)
│   ├── sliding_masks.c  # Sliding piece attack lookups
│   └── helper.c         # Debugging utilities (print board, etc.)
├── ref/
│   ├── colSelector.py   # Development helpers
│   └── rowSelector.py
├── Makefile
└── README.md
````

# Build & Run

### Prerequisites

  * GCC or Clang
  * Make

### Build

```bash
mkdir -p bin
make        # Standard build
make rel    # Release build (Optimized -Ofast -flto)
make win    # Cross-compile for Windows (requires MinGW)
```

### Run

```bash
./bin/echo
```

# UCI Support

Echo integrates with any UCI-compliant GUI (e.g., Arena, CuteChess, En Croissant).

**Supported Commands:**

```text
uci
isready
ucinewgame
position startpos | position fen <FEN>
go depth <N> | go movetime <ms>
stop
quit
```

# Reference Table

This table maps core engine concepts to their specific implementation files in the new structure.

| Concept              | File Location       | Key Functions                       |
| -------------------- | ------------------- | ----------------------------------- |
| **Search Logic** | `src/search.c`      | `negamax`, `search_position` |
| **Quiescence** | `src/search.c`      | `quiescence_search` |
| **Evaluation** | `src/eval.c`        | `eval()` |
| **Move Generation** | `src/move_gen.c`    | `generate_moves`, `make_move` |
| **Transposition Table**| `src/tt.c`        | `probeTT`, `storeTT` |
| **Zobrist Hashing** | `src/tt.c`          | `init_hash_keys`, `update_hash_key` |
| **UCI Protocol** | `src/uci.c`         | `parse_position`, `parse_go` |
| **Magic Bitboards** | `src/magic.c`       | `find_magic_number` |
| **Board Rep** | `src/board.c`       | `parse_fen`, `bitboards[]` |
| **LMR & Pruning** | `src/search.c`      | inside `negamax` loop |
| **Move Ordering** | `src/search.c`      | `score_move`, `sort_moves` |

# Debug & Dev Tools

Utility functions found in `src/helper.c` and `src/board.c`:

  * `print_board()` — ASCII/Unicode board visualization
  * `print_bitboard()` — Visualizes specific bitmasks
  * `print_move_list()` — Lists generated moves for the current state

# Credits

Echo was built on the shoulders of these giants:

  * **Maksim Korzh (Code Monkey King)**: His engine series (BBC) was foundational.
  * **Sebastian Lague**: His chess-AI educational content and source code heavily influenced the search architecture.
  * **BluefeverSoftware (VICE engine)**: Provided practical, production-grade patterns for UCI and search structures.

A huge thanks to all three — their contributions have shaped the modern hobbyist chess-engine community.
