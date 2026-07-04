# Echo Chess Engine Documentation

Echo is a high-performance, Universal Chess Interface (UCI) compliant chess engine written in C. It employs a classical architecture centered around bitboard representation, alpha-beta search, and a hand-crafted evaluation function. The development of Echo was heavily influenced by the techniques and educational resources provided by the chess programming community, particularly Maksim Korzh, Sebastian Lague, and Bluefever Software.

---

## Board Representation: Bitboards

Echo utilizes bitboards as its primary board representation. Each piece type and color is represented by a 64-bit integer (`uint64_t`), where each bit corresponds to a square on the board.

### Key Advantages
- **Parallelism:** Bitwise operations (`AND`, `OR`, `XOR`) allow for the simultaneous calculation of attacks across multiple squares.
- **Efficiency:** Native CPU instructions such as `__builtin_popcountll` (population count) and `__builtin_ctzll` (count trailing zeros) are used for fast piece counting and square indexing.
- **Hybrid Approach:** A "mailbox" array is maintained alongside bitboards to provide O(1) piece lookup for a given square, a technique often utilized in engines like VICE to optimize move generation and evaluation.

---

## Move Generation: Magic Bitboards

For sliding pieces (Rooks, Bishops, and Queens), Echo implements Magic Bitboards. This technique uses pre-calculated magic numbers and bitwise hashing to map occupancy bitmasks to pre-computed attack arrays.

- **Leaper Pieces:** Knight and King attacks are retrieved via pre-calculated look-up tables.
- **Pawn Logic:** Pawn moves are generated using bitshifts, with specialized handling for double-pushes, promotions, and en passant.
- **Legality:** The engine generates pseudo-legal moves and verifies legality during the `make_move` process by checking for king exposure.

---

## Search Architecture

The search engine is built on a Negamax framework with Alpha-Beta pruning. It incorporates several advanced refinements to achieve competitive playing strength.

### Search Refinements
- **Iterative Deepening:** The engine searches progressively deeper plies, allowing the best move from previous iterations to inform move ordering in subsequent ones.
- **Aspiration Windows:** Reduces the search space by initially searching within a small score margin around the previous iteration's result.
- **Principal Variation Search (PVS):** Optimizes the search by assuming the first move (the PV move) is likely the best and searching subsequent moves with a null-window to confirm their inferiority.

### Pruning and Reductions
- **Null Move Pruning (NMP):** Prunes branches where giving the opponent an extra move still results in a superior position.
- **Reverse Futility Pruning (RFP):** Prunes nodes where the static evaluation exceeds the beta bound by a significant margin.
- **Late Move Reductions (LMR):** Moves that are ranked lower by the move ordering heuristics are searched at a reduced depth.
- **Quiescence Search:** Mitigates the "horizon effect" by continuing the search until a stable (quiet) position is reached, focusing primarily on captures.

### Move Ordering and Heuristics
- **Transposition Table (TT):** Utilizes Zobrist Hashing to store and retrieve previously evaluated positions, preventing redundant calculations.
- **Killer Moves & History Heuristic:** Prioritizes quiet moves that have caused beta cutoffs in other branches of the search tree.
- **MVV-LVA:** Orders captures by prioritizing the highest-value victim attacked by the lowest-value attacker.

---

## Evaluation Function

Echo's evaluation function (`eval()`) is a Tapered Evaluation that interpolates between middlegame and endgame parameters based on the current phase of the game.

### Evaluated Factors
1.  **Material Balance:** Based on PeSTO's piece values.
2.  **Piece-Square Tables (PST):** Context-aware positional values for each piece type, encouraging development in the opening and king activity in the endgame.
3.  **Pawn Structure:** Rewards passed and connected pawns while penalizing isolated and doubled pawns.
4.  **Mobility:** Measures the number of squares available to each piece.
5.  **King Safety:** Evaluates the pawn shield, detects enemy "batteries," and assesses king exposure.
6.  **Mop-up Evaluation:** In the endgame, encourages driving the opponent's king toward the board edges to facilitate checkmate.

---

## UCI Support and Integration

Echo is fully compatible with the UCI protocol, allowing it to be used with standard chess GUIs (e.g., Arena, CuteChess). It supports comprehensive time management, ponder mode, and analysis features.

### Credits
Echo stands on the shoulders of giants in the chess programming community. Its design was informed by:
- **Maksim Korzh (Code Monkey King):** Foundation for bitboard and move generation logic.
- **Sebastian Lague:** Conceptual clarity on search algorithms and data structures.
- **Bluefever Software:** Practical implementation patterns for UCI and engine architecture.
- **Chess Programming Wiki:** An invaluable resource for algorithmic reference.
