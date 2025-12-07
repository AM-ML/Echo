<p align="center">
<img src="https://github.com/user-attachments/assets/0c6d308f-def4-4b25-adae-9cb41cf868d5" width="240" alt="Echo logo">
</p>

<h1 align="center"><b>E C H O — A Bitboard Chess Engine</b></h1>
<p align="center"><sub>~2430 Elo · C · UCI · Alpha–Beta · Magic Bitboards</sub></p>

<p align="center">
  <img alt="language" src="https://img.shields.io/badge/language-C-blue">
  <img alt="platform" src="https://img.shields.io/badge/platform-UCI-yellow">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-lightgrey">
  <img alt="status" src="https://img.shields.io/badge/status-active-black">
</p>

# Introduction

Echo is a compact C bitboard chess engine focused on performance, and a classical alpha–beta search architecture.
# Key Concepts

- Zobrist hashing  
- Transposition table (TT)  
- Negamax + alpha–beta pruning  
- Principal Variation Search (PVS)
- PST Interpolation
- Futility & Reverse Futility Pruning
- Null move pruning  
- Quiescence search  
- Late Move Reductions (LMR)  
- Search extensions  
- Killer & history heuristics  
- Iterative deepening  
- Move ordering (TT, PV, captures, killers, history)
- Pawn structure evaluation
- King safety
- piece mobility
- Aspiration windows  
- Threefold repetition detection  
- UCI protocol support

# Project Structure

```

/
├── src/
│   └── echo.c            # Complete engine (search, eval, UCI)
├── ref/
│   ├── colSelector.py    # Development helpers
│   └── rowSelector.py
├── Makefile
└── README.md

````

# Build & Run

## Prepare

```bash
mkdir -p bin
````

## Build

```bash
make       # Normal build
make rel   # Release build with optimizations
make win   # Cross-compile for Windows
```

## Run

```bash
./bin/echo
```

# UCI Support

Supported commands:

```
uci
isready
ucinewgame
position startpos | position fen <FEN>
go depth <N> | go movetime <ms>
stop
quit
```

Outputs standard `info` lines and `bestmove`.

# Reference Table

| Concept              | Key Functions / Location            |
| -------------------- | ----------------------------------- |
| Zobrist hashing      | `init_hash_keys`, `update_hash_key` |
| Transposition table  | `probeTT`, `storeTT`, `probe_move`  |
| Iterative deepening  | `search_position`                   |
| Negamax              | `negamax`                           |
| Quiescence           | `quiescence_search`                 |
| Null move pruning    | in `negamax`                        |
| LMR                  | in `negamax`                        |
| PVS                  | in `negamax`                        |
| Move ordering        | `score_move`, `sort_moves`          |
| Killer heuristic     | move loop                           |
| History heuristic    | move loop                           |
| Repetition detection | `is_repetition`                     |
| Evaluation           | `eval()`                            |
| UCI                  | `parse_position`, `parse_go`        |

# Debug & Dev Tools

* `print_board()` — ASCII/Unicode board
* `print_bitboard()`
* `print_move()` / `print_move_list()`
* `print_attacked_squares_by()`

# Credits

Echo was built on the shoulders of these giants:
* <b>Maksim Korzh (Code Monkey King)</b>: His engine series (BBC) was extremely useful.
* <b>Sebastian Lague</b>: His chess-AI educational videos and source code influenced search architecture (very entertaining).
* <b>BluefeverSoftware (VICE engine)</b> The VICE series provided practical, production-grade patterns for UCI + search structure.

A huge thanks to all three — their contributions shaped the modern hobbyist chess-engine community.
