<p align="center">
  <img src="https://private-user-images.githubusercontent.com/131138888/356844742-f43bb227-cb3c-4249-ad9b-49b121f9b383.png?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NjQwMjY2ODIsIm5iZiI6MTc2NDAyNjM4MiwicGF0aCI6Ii8xMzExMzg4ODgvMzU2ODQ0NzQyLWY0M2JiMjI3LWNiM2MtNDI0OS1hZDliLTQ5YjEyMWY5YjM4My5wbmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjUxMTI0JTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI1MTEyNFQyMzE5NDJaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT1mMjFmZTQ1NTdkOTE5ZmQ5MmM3MTNmYWM5OGM0ODFmZmQwMDhmZjk0NGRkNDRmMGZhYTA1MjdjM2U0OGY5YzEyJlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.uv7pTETXSlARbMSTW6P7BIXApS5IRvG0Fy_x2xP3zaw" width="240" alt="Echo logo">
</p>

<h1 align="center"><b>E C H O — A Bitboard Chess Engine</b></h1>
<p align="center"><sub>~2200 Elo · C · UCI · Alpha–Beta · Magic Bitboards</sub></p>

<p align="center">
  <img alt="language" src="https://img.shields.io/badge/language-C-blue">
  <img alt="platform" src="https://img.shields.io/badge/platform-UCI-yellow">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-lightgrey">
  <img alt="status" src="https://img.shields.io/badge/status-active-black">
</p>

# Introduction

Echo is a compact C bitboard chess engine focused on clarity, performance, and a classical alpha–beta search architecture. This README documents the engine's search, evaluation, pruning techniques, and UCI interface.

# Key Concepts

- Zobrist hashing  
- Transposition table (TT)  
- Negamax + alpha–beta pruning  
- Principal Variation Search (PVS)  
- Null move pruning  
- Quiescence search  
- Late Move Reductions (LMR)  
- Search extensions  
- Killer & history heuristics  
- Iterative deepening  
- Move ordering (TT, PV, captures, killers, history)  
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

# Search Flow (compact)

```
search_position
  └─ Iterative Deepening
      └─ Negamax
          ├─ Move Ordering (TT → PV → Captures → Killers → History)
          ├─ Alpha–Beta / PVS
          ├─ LMR
          ├─ Null Move Pruning
          └─ Quiescence
```

# Debug & Dev Tools

* `print_board()` — ASCII/Unicode board
* `print_bitboard()`
* `print_move()` / `print_move_list()`
* `print_attacked_squares_by()`

# Credits

Echo was built using insights & inspiration from: 
* <b>Maksim Korzh (Code Monkey King)</b>: His engine series (BBC) and explanations were definitely foundational.
* <b>Sebastian Lague</b>: His chess-AI educational videos and source code influenced search architecture.
* <b>BluefeverSoftware (VICE engine)</b> The VICE series provided practical, production-grade patterns for UCI + search structure.

A huge thanks to all three — their contributions shaped the modern hobbyist chess-engine community.
