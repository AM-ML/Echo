#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef _WIN32
#define FORCE_ASCII 1
#else
#define FORCE_ASCII 0
#endif

#define INF 1000000
#define NEG_INF -1000000

#define INFO(output, ...) (printf(#output "\n", __VA_ARGS__))
#define out(output) (printf(#output "\n"))

// define bitboard data type
#define U64 unsigned long long

// rank and file to square
#define RF_2SQ(r, f) (r * 8 + f)

// FEN CONSTANTS
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position                                                         \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position                                                        \
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position                                                        \
  "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position                                                           \
  "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define promotion_position "8/P5r1/8/8/8/8/3nppRP/5k1K w - - 0 2"
#define positional_position "r1b2rk1/ppp1bppp/4pn2/4P3/2B1P3/2N5/PP3PPP/R1BR2K1 b - - 0 10"

// Big Endian File-Rank Mapping
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1,
  no_square
};

enum { white, black, both };
enum { rook, bishop };
enum { wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { WCK = 1, WCQ = 2, BCK = 4, BCQ = 8 };

// switched black pieces to be used for white and same for black pieces
// due to better visual appearance
char *unicode_pieces[12] = {"♟", "♞", "♝", "♜", "♛", "♚",
                            "♙", "♘", "♗", "♖", "♕", "♔"};
char ascii_pieces[12] = "PNBRQKpnbrqk";
int decode_ascii_pieces[] = {
    ['P'] = wP, ['N'] = wN, ['B'] = wB, ['R'] = wR, ['Q'] = wQ, ['K'] = wK,
    ['p'] = bP, ['n'] = bN, ['b'] = bB, ['r'] = bR, ['q'] = bQ, ['k'] = bK,
};

U64 bitboards[12];        // pieces bbs
U64 sides_occupancies[3]; // sides

int side_to_move = -1;

int can_castle; // WCK WCQ BCQ BCK

int en_passant = no_square;

U64 hash_key; // the final hash key used to hash a position

#define REP_TABLE_SIZE 2048
U64 repetition_table[REP_TABLE_SIZE];
int repetition_index = 0;

// position repetition detection
static inline int is_repetition()
{
    // loop over repetition indicies range
    for (int index = 0; index < repetition_index; index++)
        // if we found the hash key same with a current
        if (repetition_table[index] == hash_key)
          return 1;

  return 0;
}

/***** Constants *****/
const char *square_to_notation[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

int char_to_square(const char *square) {
  if (!square) return no_square;

  char file = square[0];
  char rank = square[1];

  /* Validate that the two chars are valid file/rank digits */
  if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
    return no_square;
  }

  int file_index = file - 'a';                 /* 0..7 */
  int rank_index = 8 - (rank - '0');          /* '8'->0, '1'->7 */
  int idx = rank_index * 8 + file_index;      /* 0..63 */

  if (idx < 0 || idx >= no_square) return no_square;
  return idx;
}

const U64 not_A_file = 18374403900871474942ULL;
const U64 not_H_file = 9187201950435737471ULL;
const U64 not_HG_file = 4557430888798830399ULL;
const U64 not_AB_file = 18229723555195321596ULL;
const U64 not_B_file = 18302063728033398269ULL;
const U64 not_G_file = 13816973012072644543ULL;
const U64 not_rank_1 = 72057594037927935ULL;
const U64 not_rank_8 = 18446744073709551360ULL;
const U64 A_file = 0x0101010101010101ULL; // 01 --> 0000 0001 -> (1) = A8, A7, A6, A5...
const U64 H_file = 0x8080808080808080ULL;

const U64 rank_1 = 0xFF00000000000000ULL;
const U64 rank_8 = 0x00000000000000FFULL;


// set/get/pop macros
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= 1ULL << (square))
#define pop_bit(bitboard, square)                                              \
  (get_bit((bitboard), (square)) ? ((bitboard) -= 1ULL << (square)) : 0)
#define count_bits(bitboard) (__builtin_popcountll(bitboard))
#define get_lsb(bitboard) ((bitboard) & -(bitboard))


/* ----------------------------------------- */
/* --- rank - file - masks stuff section --- */
/* ----------------------------------------- */

#define get_file(square) ((square) % 8) // s = 8r + f --> 8r % 8 = 0, since file < 8, remainder = file
#define get_rank(square) ((square) / 8) // sqr / 8 = rank.file, remainder of that is cutoff in an integer
#define file_mask(square) (A_file << (get_file(square)))
#define rank_mask(square) (rank_1 >> 8 * (7 - get_rank[square]))


// use a lookup table since division operations are expensive.
const int get_rank[64] = {
  7, 7, 7, 7, 7, 7, 7, 7,
  6, 6, 6, 6, 6, 6, 6, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0
};

U64 pawns_file_mask[64]; // square lookup table for pawn file mask
U64 pawns_rank_mask[64]; // square lookup table for pawn rank mask
U64 isolated_pawns_mask[64];
U64 passed_pawns_mask[2][64];

const int double_pawn_penalty = -15; // will apply twice
const int isolated_pawn_penalty = -5;

const int RookOpenFileBonus = 15; // 5 since semi-open file bonus is also added (total: 15)
const int RookSemiOpenFileBonus = 10;

const int UnShieldedKingPenalty = 15;
const int SemiShieldedKingPenalty = 10;

const int ShieldedKingBonus = 5;
const int KingShieldBonus = 5; // +5 for each pawn defending the king

// the closer you are to promotion, the better
const int passed_pawn_bonus[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };

// 0 0 1 _ 1 0 0 0
static inline U64 isolated_pawn_mask(int square) {
  U64 fmask = file_mask(square);

  if (fmask == H_file) return fmask >> 1;
  if (fmask == A_file) return fmask << 1;

  return fmask << 1 | fmask >> 1;
}

static inline U64 passed_pawn_mask(int side, int square) {
  U64 fmask = file_mask(square) | isolated_pawn_mask(square);

  // since rank is inversed, 8 - rank will get it back to normal
  // each >> 8 will shift up by 1, so >> rank * 8 will shift up to the rank
  int WhiteVerticalShifter = (get_rank[square]) * 8;
  int BlackVerticalShifter = (7 - get_rank[square]) * 8;
  return (side == white)? fmask >> WhiteVerticalShifter : fmask << BlackVerticalShifter;
}


void init_pawns_eval_masks() {
  // Loop over every square on the board
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      // Initialize standard masks
      pawns_file_mask[square] = file_mask(square);
      pawns_rank_mask[square] = rank_mask(square);
      isolated_pawns_mask[square] = isolated_pawn_mask(square);

      // Reset passed pawn masks
      passed_pawns_mask[white][square] = 0ULL;
      passed_pawns_mask[black][square] = 0ULL;

      // --- WHITE PASSED PAWN MASK ---
      // White moves "up" towards Rank 0.
      // We check Ranks 0 to (current_rank - 1).
      for (int r = 0; r < rank; r++) {
        // Check left file, current file, right file
        for (int f = file - 1; f <= file + 1; f++) {
          if (f >= 0 && f <= 7) { // Ensure file is on board
            set_bit(passed_pawns_mask[white][square], RF_2SQ(r, f));
          }
        }
      }

      // --- BLACK PASSED PAWN MASK ---
      // Black moves "down" towards Rank 7.
      // We check Ranks (current_rank + 1) to 7.
      for (int r = rank + 1; r < 8; r++) {
        // Check left file, current file, right file
        for (int f = file - 1; f <= file + 1; f++) {
          if (f >= 0 && f <= 7) { // Ensure file is on board
            set_bit(passed_pawns_mask[black][square], RF_2SQ(r, f));
          }
        }
      }
    }
  }
}


static inline int get_lsb_index(U64 bitboard) {
  return bitboard ? __builtin_ctzll(bitboard) : -1;
}
void reset_states_and_board() {
  memset(bitboards, 0ULL, 96);
  memset(sides_occupancies, 0ULL, 24);

  memset(repetition_table, 0ULL, sizeof(repetition_table));
  repetition_index = 0;

  can_castle = 0;
  en_passant = no_square;
  side_to_move = -1;
}
void set_sides_occupancies() {
  sides_occupancies[white] = bitboards[wP] | bitboards[wN] | bitboards[wB] |
                             bitboards[wR] | bitboards[wQ] | bitboards[wK];
  sides_occupancies[black] = bitboards[bP] | bitboards[bN] | bitboards[bB] |
                             bitboards[bR] | bitboards[bQ] | bitboards[bK];
  sides_occupancies[both] = sides_occupancies[white] | sides_occupancies[black];
}


// print bitboard
void print_bitboard(U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8 - rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("1 ");
      }

      else {
        printf("0 ");
      }
    }

    printf("\n");
  }

  printf("    _______________\n");
  printf("    A B C D E F G H\n"); // for navigation

#else
  // print position id
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n", bitboard);

  // loop over ranks / rows
  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank); // for navigation

    // loop over files / columns
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("\033[1;91m1 ");
      }

      else {
        printf("\033[1;96m0 ");
      }
    }

    // seperate ranks
    printf("\n");
  }

  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n"); // for navigation
#endif
}

void print_sides_occupancies() {
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n",
         sides_occupancies[both]);

  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank);
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);
      if (get_bit(sides_occupancies[white], square)) {
        printf("\033[1;94m1 ");
      } else if (get_bit(sides_occupancies[black], square)) {
        printf("\033[1;91m2 ");
      } else {
        printf("\033[1;96m0 ");
      }
    }
    printf("\n");
  }
  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n");
}

void print_bitboard_piece(int piece_square, U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8 - rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (square == piece_square) {
        printf("P ");
      }

      else if (get_bit(bitboard, square)) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }

    printf("\n");
  }

  printf("    _______________\n");
  printf("    A B C D E F G H\n"); // for navigation

#else
  // print position id
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n", bitboard);

  // loop over ranks / rows
  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank); // for navigation

    // loop over files / columns
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (square == piece_square) {
        printf("\033[1;93mP ");
      }

      else if (get_bit(bitboard, square)) {
        printf("\033[1;91m1 ");
      }

      else {
        printf("\033[1;96m0 ");
      }
    }

    // seperate ranks
    printf("\n");
  }

  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n"); // for navigation
#endif
}

void print_board(int flag) {
  if (FORCE_ASCII) {
    // Windows / ASCII mode
    printf("\nPosition: %llu\n", sides_occupancies[both]);
    printf("Castling: %c%c%c%c\n",
           can_castle & WCK ? 'K' : '_',
           can_castle & WCQ ? 'Q' : '_',
           can_castle & BCK ? 'k' : '_',
           can_castle & BCQ ? 'q' : '_');
    printf("En Passant: %s\n\n",
           en_passant != no_square ? square_to_notation[en_passant] : "_");
    if (side_to_move != -1)
      printf("%s To Move\n", side_to_move == white ? "White" : "Black");
  } else {
    // Colored Unicode mode
    printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m",
           sides_occupancies[both]);
    printf("\n\033[1;93mHash Key: \033[1;95m%llx\033[0;0m",
           hash_key);
    printf("\n\033[1;93mCastling: \033[1;95m%c%c%c%c\033[0;0m",
           can_castle & WCK ? 'K' : '_',
           can_castle & WCQ ? 'Q' : '_',
           can_castle & BCK ? 'k' : '_',
           can_castle & BCQ ? 'q' : '_');
    printf("\n\033[1;93mEn Passant: \033[1;95m%s\033[0;0m\n\n",
           en_passant != no_square ? square_to_notation[en_passant] : "_");
    if (side_to_move != -1)
      printf("%s\033[1;93m To Move\033[0;0m\n",
             side_to_move == white ? "\033[1;96mWhite" : "\033[36mBlack");
  }

  for (int rank = 0; rank < 8; rank++) {
    // top border of row
    printf("   ");
    for (int file = 0; file < 8; file++) printf("+---");
    printf("+\n");

    // rank label
    if (FORCE_ASCII)
      printf(" %d |", 8 - rank);
    else
      printf(" \033[1;93m%d\033[0;0m |", 8 - rank);

    // print pieces
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);
      char *c = " ";   // empty square
      char *color = ""; // default no color

      if (get_bit(sides_occupancies[both], square)) {
        if (!FORCE_ASCII && flag) {
          // Unicode mode
          if (get_bit(bitboards[wP], square))
            c = unicode_pieces[wP], color = "\033[1;96m";
          else if (get_bit(bitboards[wB], square))
            c = unicode_pieces[wB], color = "\033[1;96m";
          else if (get_bit(bitboards[wN], square))
            c = unicode_pieces[wN], color = "\033[1;96m";
          else if (get_bit(bitboards[wR], square))
            c = unicode_pieces[wR], color = "\033[1;96m";
          else if (get_bit(bitboards[wQ], square))
            c = unicode_pieces[wQ], color = "\033[1;96m";
          else if (get_bit(bitboards[wK], square))
            c = unicode_pieces[wK], color = "\033[1;96m";

          else if (get_bit(bitboards[bP], square))
            c = unicode_pieces[bP], color = "\033[36m";
          else if (get_bit(bitboards[bB], square))
            c = unicode_pieces[bB], color = "\033[36m";
          else if (get_bit(bitboards[bN], square))
            c = unicode_pieces[bN], color = "\033[36m";
          else if (get_bit(bitboards[bR], square))
            c = unicode_pieces[bR], color = "\033[36m";
          else if (get_bit(bitboards[bQ], square))
            c = unicode_pieces[bQ], color = "\033[36m";
          else if (get_bit(bitboards[bK], square))
            c = unicode_pieces[bK], color = "\033[36m";
        } else {
          // ASCII mode
          if (get_bit(bitboards[wP], square))
            c = "P";
          else if (get_bit(bitboards[wB], square))
            c = "B";
          else if (get_bit(bitboards[wN], square))
            c = "N";
          else if (get_bit(bitboards[wR], square))
            c = "R";
          else if (get_bit(bitboards[wQ], square))
            c = "Q";
          else if (get_bit(bitboards[wK], square))
            c = "K";
          else if (get_bit(bitboards[bP], square))
            c = "p";
          else if (get_bit(bitboards[bB], square))
            c = "b";
          else if (get_bit(bitboards[bN], square))
            c = "n";
          else if (get_bit(bitboards[bR], square))
            c = "r";
          else if (get_bit(bitboards[bQ], square))
            c = "q";
          else if (get_bit(bitboards[bK], square))
            c = "k";
        }
      }

      // print square
      if (!FORCE_ASCII && flag)
        printf(" %s%s\033[0;0m |", color, c);
      else
        printf(" %s |", c);
    }
    printf("\n");
  }

  // bottom border
  printf("   ");
  for (int file = 0; file < 8; file++) { printf("+---"); }
  printf("+\n");

  // file letters
  if (FORCE_ASCII)
    printf("     A   B   C   D   E   F   G   H\n");
  else
    printf("     \033[1;93mA   B   C   D   E   F   G   H\033[0;0m\n");
}







/**** Attacks ****/

/*** Pawns ***/

// pawn attacks table:: [sides][squares]
U64 pawn_attacks[2][64];

// pawn attacks generator function
U64 mask_pawn_attacks(int side, int square) {
  U64 attacks = 0ULL; // attacks bitboard

  U64 bitboard = 0ULL;       // piece bitboard
  set_bit(bitboard, square); // set piece on bitboard

  // white side
  if (!side) {
    // if the right pawn attack square is not on A file (not possible)
    if ((bitboard >> 7) & not_A_file)
      attacks |= bitboard >> 7;
    // if the left pawn attack square is not on H file (not possible)
    if ((bitboard >> 9) & not_H_file)
      attacks |= bitboard >> 9;
  }
  // black side
  else {
    if ((bitboard << 7) & not_H_file)
      attacks |= bitboard << 7;
    if ((bitboard << 9) & not_A_file)
      attacks |= bitboard << 9;
  }

  return attacks;
}

/*** Knights ***/
U64 knight_attacks[64];

U64 mask_knight_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if (bitboard << 6 & not_HG_file)
    attacks |= bitboard << 6;
  if (bitboard << 10 & not_AB_file)
    attacks |= bitboard << 10;
  if (bitboard << 15 & not_H_file)
    attacks |= bitboard << 15;
  if (bitboard << 17 & not_A_file)
    attacks |= bitboard << 17;

  if (bitboard >> 6 & not_AB_file)
    attacks |= bitboard >> 6;
  if (bitboard >> 10 & not_HG_file)
    attacks |= bitboard >> 10;
  if (bitboard >> 15 & not_A_file)
    attacks |= bitboard >> 15;
  if (bitboard >> 17 & not_H_file)
    attacks |= bitboard >> 17;

  return attacks;
}

/*** King ***/
U64 king_attacks[64];

U64 mask_king_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if ((bitboard << 8))
    attacks |= bitboard << 8;
  if ((bitboard << 9) & not_A_file)
    attacks |= bitboard << 9;
  if ((bitboard << 7) & not_H_file)
    attacks |= bitboard << 7;
  if ((bitboard << 1) & not_A_file)
    attacks |= bitboard << 1;

  if ((bitboard >> 8))
    attacks |= bitboard >> 8;
  if ((bitboard >> 9) & not_H_file)
    attacks |= bitboard >> 9;
  if ((bitboard >> 7) & not_A_file)
    attacks |= bitboard >> 7;
  if ((bitboard >> 1) & not_H_file)
    attacks |= bitboard >> 1;

  return attacks;
}

/**** bishop ****/

U64 bishop_masks[64];
U64 bishop_attacks[64][512]; // 512: max occupancy index for bishops

U64 mask_bishop_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  // mask relevant bishop occupancy bits
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++)
    attacks |= (1ULL << (RF_2SQ(r, f)));
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--)
    attacks |= (1ULL << (RF_2SQ(r, f)));
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--)
    attacks |= (1ULL << (RF_2SQ(r, f)));
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++)
    attacks |= (1ULL << (RF_2SQ(r, f)));

  return attacks;
}

U64 relevant_bishop_attacks(int square, U64 block) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  // mask relevant bishop occupancy bits + board edge
  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
    attacks |= (1ULL << (RF_2SQ(r, f))); // add attack square
    if ((1ULL << (RF_2SQ(r, f))) & block)
      break; // then break, indicate piece can be captured
  }

  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block)
      break;
  }

  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block)
      break;
  }

  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block)
      break;
  }

  return attacks;
}

/**** rook ****/
U64 rook_masks[64];
U64 rook_attacks[64][4096]; // 4096: max occupancy index for rooks

U64 mask_rook_attacks(int square) {
  U64 attacks = 0ULL;

  int r, f;
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  for (r = tr + 1; r < 7; r++)
    attacks |= (1ULL << (RF_2SQ(r, tf)));
  for (r = tr - 1; r > 0; r--)
    attacks |= (1ULL << (RF_2SQ(r, tf)));
  for (f = tf + 1; f < 7; f++)
    attacks |= (1ULL << (RF_2SQ(tr, f)));
  for (f = tf - 1; f > 0; f--)
    attacks |= (1ULL << (RF_2SQ(tr, f)));

  return attacks;
}

U64 relevant_rook_attacks(int square, U64 block) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  for (r = tr + 1; r <= 7; r++) {
    attacks |= (1ULL << (RF_2SQ(r, tf)));
    if ((1ULL << (RF_2SQ(r, tf))) & block)
      break;
  }

  for (r = tr - 1; r >= 0; r--) {
    attacks |= (1ULL << (RF_2SQ(r, tf)));
    if ((1ULL << (RF_2SQ(r, tf))) & block)
      break;
  }

  for (f = tf + 1; f <= 7; f++) {
    attacks |= (1ULL << (RF_2SQ(tr, f)));
    if ((1ULL << (RF_2SQ(tr, f))) & block)
      break;
  }

  for (f = tf - 1; f >= 0; f--) {
    attacks |= (1ULL << (RF_2SQ(tr, f)));
    if ((1ULL << (RF_2SQ(tr, f))) & block)
      break;
  }

  return attacks;
}

void init_leaper_attacks() {
  for (int square = 0; square < 64; square++) {
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    knight_attacks[square] = mask_knight_attacks(square);
    king_attacks[square] = mask_king_attacks(square);
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);
  }
}

/**** RELEVANT BIT COUNT LOOKUP TABLE ****/
const int relevant_bishop_count_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
    5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7,
    7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

const int relevant_rook_count_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

const int relevant_knight_count_bits[64] = {
    2, 3, 4, 4, 4, 4, 3, 2, 3, 4, 6, 6, 6, 6, 4, 3, 4, 6, 8, 8, 8, 8,
    6, 4, 4, 6, 8, 8, 8, 8, 6, 4, 4, 6, 8, 8, 8, 8, 6, 4, 4, 6, 8, 8,
    8, 8, 6, 4, 3, 4, 6, 6, 6, 6, 4, 3, 2, 3, 4, 4, 4, 4, 3, 2};

const int relevant_queen_count_bits[64] = {
    18, 16, 16, 16, 16, 16, 16, 18, 16, 15, 15, 15, 15, 15, 15, 16,
    16, 15, 17, 17, 17, 17, 15, 16, 16, 15, 17, 19, 19, 17, 15, 16,
    16, 15, 17, 19, 19, 17, 15, 16, 16, 15, 17, 17, 17, 17, 15, 16,
    16, 15, 15, 15, 15, 15, 15, 16, 18, 16, 16, 16, 16, 16, 16, 18};

U64 rook_magic_numbers[64] = {
    36046528791461889ULL,   306245049807544320ULL,  36063983539585032ULL,
    36046391353016324ULL,   4647723611581318144ULL, 144119654858752008ULL,
    36029346791555584ULL,   144115465103474948ULL,  2392539453718560ULL,
    2305984296994750464ULL, 864831934665064577ULL,  4620834023965460480ULL,
    576602039681811456ULL,  9147945333293184ULL,    562954382901760ULL,
    9232519977892855936ULL, 1188950576505815104ULL, 72198881818984451ULL,
    18692788199424ULL,      282574756782088ULL,     3467915199476925440ULL,
    563499776376834ULL,     153126785512374785ULL,  580966551230890244ULL,
    612630426397196288ULL,  70369820020736ULL,      35186527965184ULL,
    4611703612769306624ULL, 9223380835095543936ULL, 562967200407568ULL,
    36873226256777220ULL,   2305843292681797761ULL, 9295500000182141056ULL,
    576531189771280384ULL,  576532220844449792ULL,  4644405843593216ULL,
    140754676615170ULL,     576462953482552320ULL,  1168298213896ULL,
    619582587905ULL,        5764748535402627072ULL, 70369281081472ULL,
    216190374836732032ULL,  36046389339259008ULL,   8800389103620ULL,
    1125908505198596ULL,    1441153014902816770ULL, 566940008449ULL,
    18014948269498496ULL,   70437465751616ULL,      2377918195974570112ULL,
    8798241554560ULL,       4647723613687120000ULL, 306385529329549440ULL,
    288511859718619392ULL,  4574417011200ULL,       35461397544977ULL,
    1407514638827521ULL,    281518196916289ULL,     72620819406652434ULL,
    9223935021436641282ULL, 281483633756161ULL,     1168243820548ULL,
    281477128397313ULL};
U64 bishop_magic_numbers[64] = {146369204096991744ULL,  1134698216587296ULL,
                                9234774526519672896ULL, 73187928636915974ULL,
                                9306541899907200ULL,    2401470968299648ULL,
                                1153203563787190304ULL, 2306478569902129168ULL,
                                4708492640384ULL,       11602416136897986688ULL,
                                9354443096064ULL,       612498483932299264ULL,
                                9224502352525000704ULL, 36029355903680512ULL,
                                2452211101044195328ULL, 72057870056955904ULL,
                                580964627081470080ULL,  9007268003645696ULL,
                                40532431010283528ULL,   45071326708252672ULL,
                                4786209012842496ULL,    1143638126102528ULL,
                                22588371511697408ULL,   288389259944034816ULL,
                                4648744022708752ULL,    1143509809762432ULL,
                                146384891594948672ULL,  2895818958449938464ULL,
                                281543712980992ULL,     140874960998400ULL,
                                283678311747584ULL,     316951423615104ULL,
                                4521260533482016ULL,    2256266579951872ULL,
                                70574970241088ULL,      145273108824320ULL,
                                74783970574592ULL,      4611985089885119490ULL,
                                1169916946948224ULL,    2308101958000771648ULL,
                                299102059365889ULL,     74775515897856ULL,
                                282025823080457ULL,     137841608960ULL,
                                8800455114816ULL,       1193612299649155136ULL,
                                1155175520628900864ULL, 2252900541661696ULL,
                                9298807898891682112ULL, 35751576731648ULL,
                                1101793591296ULL,       545800192ULL,
                                2306265506493038592ULL, 4611757555436257280ULL,
                                2269410261991424ULL,    1143500689178760ULL,
                                73254008854152256ULL,   344805869568ULL,
                                4303881216ULL,          4611706016336314880ULL,
                                21990503096840ULL,      141801095680ULL,
                                2305847441654022696ULL, 144695738814432384ULL};

/**** OCCUPANCY AND MAGIC SECTION ****/

// creates attack mask and maps each occupied square to a bit
// i.e: 1 = first occupied square from top left to bottom right
// 2 = 2nd, 3 = 1st + 2nd, 4 = 3rd, 5 = 3rd + 1st, 6 = 3rd + 2nd + 1st etc..
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; count++) {
    int square = get_lsb_index(attack_mask); // from top left to bottom right
    pop_bit(attack_mask, square);

    if ((U64)index & (1ULL << count)) { // if square in specified bit array
      set_bit(occupancy, square);       // add bit
    }
  }

  return occupancy;
}

/**** pseudo random number state ****/
unsigned int state = 1804289383;

unsigned int get_random_32() {
  unsigned int number = state;
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  state = number;

  return number;
}

U64 get_random_64() {
  U64 n1, n2, n3, n4;

  n1 = (U64)(get_random_32()) & 0XFFFF; // slice 16 bits from MS1B side
  n2 = (U64)(get_random_32()) & 0XFFFF;
  n3 = (U64)(get_random_32()) & 0XFFFF;
  n4 = (U64)(get_random_32()) & 0XFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 gen_magic_number() {
  return get_random_64() & get_random_64() & get_random_64() & get_random_64();
}

U64 find_magic_number(int square, int relevant_bits_count, int flag) {
  U64 occupancies[4096]; // max: 4096 bytes or 12 occupied squares for rook

  U64 attacks[4096]; // max: same as occupied

  U64 used_attacks[4096];

  U64 attack_mask =
      flag ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  U64 occupancy_indicies = 1 << relevant_bits_count;

  for (int index = 0; index < occupancy_indicies;
       index++) { // loop over indicies
    occupancies[index] = set_occupancy(index, relevant_bits_count,
                                       attack_mask); // store each possibility

    attacks[index] =
        flag ? relevant_bishop_attacks(square, occupancies[index]) //
             : relevant_rook_attacks(square, occupancies[index]);
  }

  for (int random_count = 0; random_count < 800000000; random_count++) {
    U64 magic_number = gen_magic_number();

    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
      continue;

    memset(used_attacks, 0ULL, sizeof(used_attacks));

    int index, fail;

    // loop over occupancy indicies
    for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {
      int magic_index = (int)((occupancies[index] * magic_number) >>
                              (64 - relevant_bits_count));

      if (used_attacks[magic_index] == 0ULL)
        used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index])
        fail = 1;
    }

    if (!fail)
      return magic_number;
  }
  printf("\b    attempt failed.\n");
  return 0ULL;
}


// ---------------------------
// ----- ZOBRIST HASHING -----
// ---------------------------

//      [piece][square]
U64 piece_keys[12][64];
U64 enpassant_keys[64]; // [square]
U64 castle_keys[16]; // 1111 KQkq = 16
U64 side_to_move_key; // white : black (0, 1)


void init_hash_keys () {
  state = 1804289383; // if constant, key generation is constant (which is good)

  for(int piece = wP; piece <= bK; piece++) { // for each piece
    for(int square = 0; square < 64; square++) { // loop over each square
      piece_keys[piece][square] = get_random_64();
    }
  }

  for(int square = 0; square < 64; square++) {
    enpassant_keys[square] = get_random_64();
  }

  for(int i = 0; i < 16; i++) {
    castle_keys[i] = get_random_64();
  }

  side_to_move_key = get_random_64();
}


U64 update_hash_key() {
  U64 fkey = 0ULL; // XORs piece position, enpassant, castling info, side_to_move
  U64 piece_bb; // temporary piece bitboard placeholder

  // XOR hashed pieces position into the final hash key
  for(int piece = wP; piece <= bK; piece++) { // loop over each piece's bitboard
    piece_bb = bitboards[piece];

    // while there is still pieces not hashed
    while(piece_bb) {
      int piece_square = get_lsb_index(piece_bb); // get piece position

      fkey ^= piece_keys[piece][piece_square]; // hash square and add it to position

      pop_bit(piece_bb, piece_square);
    }
  }

  // if en passant square is in position
  if (en_passant != no_square) {
    fkey ^= enpassant_keys[en_passant]; // XOR hashed enpassant square
  }

  fkey ^= castle_keys[can_castle];
  if (side_to_move == black) fkey ^= side_to_move_key;

  return fkey;
}

// order: 8/7/6/5/4/3/2/1 (top to bottom) | 12345678 (left to right) /12345678
void parse_fen(char *fen) {
  reset_states_and_board();

  // Parse board position
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8;) {
      int square = RF_2SQ(rank, file);

      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        int piece = decode_ascii_pieces[*fen++];
        set_bit(bitboards[piece], square);
        file++;
      } else if (*fen >= '1' && *fen <= '8') {
        file += *fen++ - '0';
      } else {
        fen++;
      }
    }
  }

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse side to move
  side_to_move = (*fen == 'w') ? white : black;
  fen++;

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse castling rights
  can_castle = 0;
  if (*fen != '-') {
    while (*fen != ' ') {
      switch (*fen++) {
      case 'K':
        can_castle |= WCK;
        break;
      case 'Q':
        can_castle |= WCQ;
        break;
      case 'k':
        can_castle |= BCK;
        break;
      case 'q':
        can_castle |= BCQ;
        break;
      }
    }
  } else {
    fen++;
  }

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse en passant square
  if (*fen == '-') {
    en_passant = no_square;
    fen++;
  } else {
    en_passant = char_to_square(fen);
    fen += 2;
  }

  // Skip remaining FEN components (ply and move count)
  while (*fen && *fen != ' ')
    fen++;

  // Finalize board states
  set_sides_occupancies();
  hash_key = update_hash_key();
}

void init_sliding_pieces(int flag) {
  for (int square = 0; square < 64; square++) {
    int is_bishop = flag == bishop;
    U64 attack_mask = is_bishop ? bishop_masks[square] : rook_masks[square];

    int relevant_bits_count = count_bits(attack_mask);

    int occupancy_indicies = 1ULL << relevant_bits_count;

    for (int index = 0; index < occupancy_indicies; index++) {
      if (is_bishop) {
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        int magic_index = (int)((occupancy * bishop_magic_numbers[square]) >>
                                (64 - relevant_bishop_count_bits[square]));

        bishop_attacks[square][magic_index] =
            relevant_bishop_attacks(square, occupancy);
      } else {
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        int magic_index = (int)((occupancy * rook_magic_numbers[square]) >>
                                (64 - relevant_rook_count_bits[square]));

        rook_attacks[square][magic_index] =
            relevant_rook_attacks(square, occupancy);
      }
    }
  }
}

// get attacks from magic index
// #define get_bishop_attacks(square, blockers) (blockers = (((blockers &
// bishop_masks[square]) * bishop_magic_numbers[square])) >> (64 -
// relevant_bishop_count_bits[square]))

// #define get_rook_attacks(square, blockers) (blockers = (((blockers &
// rook_masks[square]) * rook_magic_numbers[square])) >> (64 -
// relevant_rook_count_bits[square]))

static inline U64 get_bishop_attacks(int square, U64 blockers) {
  blockers &= bishop_masks[square];
  blockers *= bishop_magic_numbers[square];
  blockers >>= 64 - relevant_bishop_count_bits[square];

  return bishop_attacks[square][blockers];
}

static inline U64 get_rook_attacks(int square, U64 blockers) {
  blockers &= rook_masks[square];
  blockers *= rook_magic_numbers[square];
  blockers >>= 64 - relevant_rook_count_bits[square];

  return rook_attacks[square][blockers];
}

static inline U64 get_queen_attacks(int square, U64 blockers) {
  U64 rook_blockers, bishop_blockers, result;
  rook_blockers = blockers;
  bishop_blockers = blockers;

  rook_blockers &= rook_masks[square];
  rook_blockers *= rook_magic_numbers[square];
  rook_blockers >>= 64 - relevant_rook_count_bits[square];

  result = rook_attacks[square][rook_blockers];

  bishop_blockers &= bishop_masks[square];
  bishop_blockers *= bishop_magic_numbers[square];
  bishop_blockers >>= 64 - relevant_bishop_count_bits[square];

  result |= bishop_attacks[square][bishop_blockers];

  return result;
}

static inline int is_square_attacked_by(int square, int side) {
  // Option: Remove 'both' support if you don't explicitly use it in search to save time.
  // If you strictly need it, keep the recursive check, but usually search uses specific sides.
  if (side == both)
      return is_square_attacked_by(square, white) || is_square_attacked_by(square, black);

  // Pawn attacks (Using the lookup table directly)
  // We check if an enemy pawn is on the attacking square relative to 'square'
  if (pawn_attacks[side ^ 1][square] & bitboards[side == white ? wP : bP]) return 1;

  // Knight attacks
  if (knight_attacks[square] & bitboards[side == white ? wN : bN]) return 1;

  // King attacks
  if (king_attacks[square] & bitboards[side == white ? wK : bK]) return 1;

  // Bishop/Queen attacks (Linear sliding)
  // We combine Bishop + Queen bitboards to check once
  U64 bq = bitboards[side == white ? wB : bB] | bitboards[side == white ? wQ : bQ];
  if (bq && (get_bishop_attacks(square, sides_occupancies[both]) & bq)) return 1;

  // Rook/Queen attacks (Linear sliding)
  U64 rq = bitboards[side == white ? wR : bR] | bitboards[side == white ? wQ : bQ];
  if (rq && (get_rook_attacks(square, sides_occupancies[both]) & rq)) return 1;

  return 0;
}

static inline U64 get_attacked_squares_by(int side) {
  U64 attack_map = 0ULL;
  for (int square = 0; square < 64; square += 4) {
    if (is_square_attacked_by(square, side))
      set_bit(attack_map, square);
    if (is_square_attacked_by(square + 1, side))
      set_bit(attack_map, square + 1);
    if (is_square_attacked_by(square + 2, side))
      set_bit(attack_map, square + 2);
    if (is_square_attacked_by(square + 3, side))
      set_bit(attack_map, square + 3);
  }
  return attack_map;
}

#define print_attacked_squares_by(side)                                        \
  (print_bitboard(get_attacked_squares_by((side))))

void init_magic_numbers() {
  for (int square = 0; square < 64; square++) {
    rook_magic_numbers[square] =
        find_magic_number(square, relevant_rook_count_bits[square], rook);
    printf("%lluULL, ", rook_magic_numbers[square]);
  }
  printf("\n");
  for (int square = 0; square < 64; square++) {
    bishop_magic_numbers[square] =
        find_magic_number(square, relevant_bishop_count_bits[square], bishop);
    printf("%lluULL, ", bishop_magic_numbers[square]);
  }
  printf("\n");
}

int bin(int p) {
  int result = 1;
  for (int i = 0; i < p; i++) {
    result *= 2;
  }
  return result;
}
void automate_occupancy(U64 mask) {
  int count = count_bits(mask);

  for (int i = 1, b = 1; i <= count; i++, b = bin(i) - 1) {
    print_bitboard(set_occupancy(b, count, mask));
  }
}

/* --- 24 bits / 3 bytes ---    Encoded Move-List Item Structure
 *
 *           BINARY                                  HEXADECIMAL
  0000 0000 0000 0000 0011 1111   source square      0x3f
  0000 0000 0000 1111 1100 0000   target square      0xfc0
  0000 0000 1111 0000 0000 0000   piece              0xf000
  0000 1111 0000 0000 0000 0000   promoted piece     0xf0000
  0001 0000 0000 0000 0000 0000   capture flag       0x100000
  0010 0000 0000 0000 0000 0000   double push flag   0x200000
  0100 0000 0000 0000 0000 0000   enpassant flag     0x400000
  1000 0000 0000 0000 0000 0000   castling flag      0x800000
*
*/

// --- move encoding macros ---
#define encode_move(source, target, piece, promoted_piece, capture,            \
                    double_push, en_passant, castling)                         \
  (source) | ((target) << 6) | ((piece) << 12) | ((promoted_piece) << 16) |    \
      ((capture) << 20) | ((double_push) << 21) | ((en_passant) << 22) |       \
      ((castling) << 23)

#define get_move_source(move) ((move) & 0x3f)
#define get_move_target(move) (((move) & 0xfc0) >> 6)
#define get_move_piece(move) (((move) & 0xf000) >> 12)
#define get_move_promoted_piece(move) (((move) & 0xf0000) >> 16)
#define get_move_capture_flag(move) (((move) & 0x100000))
#define get_move_double_push_flag(move) (((move) & 0x200000))
#define get_move_en_passant_flag(move) (((move) & 0x400000))
#define get_move_castling_flag(move) (((move) & 0x800000))

static inline int is_valid_encoded_move(int move) {
    int src = get_move_source(move);
    int dst = get_move_target(move);
    int pc  = get_move_piece(move);

    /* simple field range checks */
    if (src < 0 || src >= 64) return 0;
    if (dst < 0 || dst >= 64) return 0;
    if (pc  < 0 || pc  >= 12) return 0; /* piece values 0..11 */
    if (move == 0) return 0; // hopefully fixes null move a8a8 bug
    return 1;
}

// --- move output debugging function
#define print_move_info(move)                                                  \
  INFO("Source Square: %s", square_to_notation[get_move_source(move)]);        \
  INFO("Target Square: %s", square_to_notation[get_move_target(move)]);        \
  INFO("Piece: %c", ascii_pieces[get_move_piece(move)]);                       \
  INFO("Promoted Piece: %c%s", ascii_pieces[get_move_promoted_piece(move)],    \
       get_move_promoted_piece(move) == 0 ? " or N/A" : "");                   \
  INFO("Castle: %d", get_move_castling_flag(move) ? 1 : 0);                    \
  INFO("Capture: %d", get_move_capture_flag(move) ? 1 : 0);                    \
  INFO("En Passant: %d", get_move_en_passant_flag(move) ? 1 : 0);              \
  INFO("Double Pawn Push: %d", get_move_double_push_flag(move) ? 1 : 0);

typedef struct {
  int moves[256]; // theoretical move limit: 255

  int count;
} Moves;

#define MOVES_CAPACITY 256

#define MOVES_CAPACITY 256
static inline int add_move(Moves *move_list, int move) {
  if (!move_list) return 0;
  if (!is_valid_encoded_move(move)) {
    /* malformed: ignore it */
    return 0;
  }
  if (move_list->count >= MOVES_CAPACITY) {
    /* capacity exceeded */
    return 0;
  }
  move_list->moves[move_list->count++] = move;
  return 1;
}

// --- add move helpers

char ascii_promoted_pieces[] = {[0] = '\0', // get_move_promoted_piece = '0000'
                                            // or 'wP' ? (illegal) => print '\0'
                                [wQ] = 'q', [wR] = 'r', [wB] = 'b', [wN] = 'n',
                                [bQ] = 'q', [bR] = 'r', [bB] = 'b', [bN] = 'n'};

// for UCI purposes
static inline void print_move(int move) {
  printf("%s%s%c\n", square_to_notation[get_move_source(move)],
         square_to_notation[get_move_target(move)],
         ascii_promoted_pieces[get_move_promoted_piece(move)]);
}
static inline char* get_move_str(int move) {
  static char buffer[8];
  snprintf(buffer, 8, "%s%s%c", square_to_notation[get_move_source(move)],
         square_to_notation[get_move_target(move)],
         ascii_promoted_pieces[get_move_promoted_piece(move)]);
  return buffer;
}

// for debugging purposes
static inline void print_move_list(Moves *move_list) {
  printf("\n");

  if (!move_list->count)
    out("No moves in the list.");

  else {
    for (int i = 0; i < move_list->count; i++) {
      int move = move_list->moves[i];

      char *capture = get_move_capture_flag(move) ? "capture   " : "";
      char *castling_f = get_move_castling_flag(move) ? "castle   " : "";
      char *en_passant_f =
          get_move_en_passant_flag(move) ? "en_passant   " : "";
      char *double_push =
          get_move_double_push_flag(move) ? "double push   " : "";


      size_t len = strlen(capture) + strlen(castling_f) + strlen(en_passant_f) + strlen(double_push) + 1;
      char *out = malloc(len);
      if (!out) { perror("malloc"); exit(EXIT_FAILURE); }
      out[0] = '\0';
      strcat(out, capture);
      strcat(out, castling_f);
      strcat(out, en_passant_f);
      strcat(out, double_push);

      printf("%c %s-%s%c\t%s\n", ascii_pieces[get_move_piece(move)],
             square_to_notation[get_move_source(move)],
             square_to_notation[get_move_target(move)],
             ascii_promoted_pieces[get_move_promoted_piece(move)], out);

      free(out);
    }

    printf("\n\033[1;94mMoves: \033[1;93m%d\033[0;0m\n\n", move_list->count);
  }
}

// generate moves function

static inline void generate_moves(Moves *moves_list) {
  moves_list->count = 0;
  int src_sqr, dest_sqr;
  U64 position, attacks;

  int base = side_to_move == white? wP : bP;
  for (int i = 0; i < 6; i++) {
    int piece = base + i;
    position = bitboards[piece];

    if (side_to_move == white) {
      if (piece == wP) {
        while (position) {
          src_sqr = get_lsb_index(position);
          dest_sqr = src_sqr - 8;

          // FIX: Check dest_sqr is within bounds (< a8 means >= 0)
          if (!(dest_sqr < a8) && !get_bit(sides_occupancies[both], dest_sqr)) {

            // FIX: Promotion happens on rank 7 (squares a7-h7 have indices 8-15)
            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 0, 0, 0, 0));
            } else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 0, 0, 0, 0));

              // FIX: Double push from rank 2 (squares a2-h2 have indices 48-55)
              if (src_sqr >= a2 && src_sqr <= h2 &&
                  !get_bit(sides_occupancies[both], dest_sqr - 8)) {
                add_move(moves_list,
                         encode_move(src_sqr, dest_sqr - 8, wP, 0, 0, 1, 0, 0));
              }
            }
          }

          attacks = pawn_attacks[white][src_sqr] & sides_occupancies[black];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            // FIX: Proper bounds checking
            if (dest_sqr < 0 || dest_sqr >= no_square) {
              pop_bit(attacks, dest_sqr);
              continue;
            }

            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 1, 0, 0, 0));
            }

            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[white][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[bP], en_passant + 8)) {

              add_move(moves_list,
                       encode_move(src_sqr, en_passant, wP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }

      if (piece == wK) {

        if (can_castle &
            WCK) {
          if (!get_bit(sides_occupancies[both], f1) &&
              !get_bit(sides_occupancies[both], g1)) {
            if (!is_square_attacked_by(e1, black) &&
                !is_square_attacked_by(f1, black) &&
                !is_square_attacked_by(g1, black)) {
              add_move(moves_list, encode_move(e1, g1, wK, 0, 0, 0, 0, 1));
            }
          }
        }

        if (can_castle & WCQ) {
          if (!get_bit(sides_occupancies[both], d1) &&
              !get_bit(sides_occupancies[both], c1) &&
              !get_bit(sides_occupancies[both], b1)) {
            if (!is_square_attacked_by(e1, black) &&
                !is_square_attacked_by(d1, black) &&
                !is_square_attacked_by(c1, black)) {
              add_move(moves_list, encode_move(e1, c1, wK, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    } else {
      if (piece == bP) {
        while (position) {
          src_sqr = get_lsb_index(position);
          dest_sqr = src_sqr + 8;

          // FIX: Check dest_sqr is within bounds (<= h1 means < 64)
          if (!(dest_sqr > h1) && !get_bit(sides_occupancies[both], dest_sqr)) {

            // FIX: Promotion happens on rank 2 (squares a2-h2 have indices 48-55)
            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 0, 0, 0, 0));
            } else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 0, 0, 0, 0));

              // FIX: Double push from rank 7 (squares a7-h7 have indices 8-15)
              if (src_sqr >= a7 && src_sqr <= h7 &&
                  !get_bit(sides_occupancies[both], dest_sqr + 8)) {
                add_move(moves_list,
                         encode_move(src_sqr, dest_sqr + 8, bP, 0, 0, 1, 0, 0));
              }
            }
          }
          attacks = pawn_attacks[black][src_sqr] & sides_occupancies[white];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            // FIX: Proper bounds checking
            if (dest_sqr < 0 || dest_sqr >= no_square) {
              pop_bit(attacks, dest_sqr);
              continue;
            }

            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 1, 0, 0, 0));
            }

            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }
          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[black][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[wP], en_passant - 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, bP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
      if (piece == bK) {

        if (can_castle &
            BCK) {
          if (!get_bit(sides_occupancies[both], f8) &&
              !get_bit(sides_occupancies[both], g8)) {
            if (!is_square_attacked_by(e8, white) &&
                !is_square_attacked_by(f8, white) &&
                !is_square_attacked_by(g8, white)) {
              add_move(moves_list, encode_move(e8, g8, bK, 0, 0, 0, 0, 1));
            }
          }
        }

        if (can_castle & BCQ) {
          if (!get_bit(sides_occupancies[both], d8) &&
              !get_bit(sides_occupancies[both], c8) &&
              !get_bit(sides_occupancies[both], b8)) {
            if (!is_square_attacked_by(e8, white) &&
                !is_square_attacked_by(d8, white) &&
                !is_square_attacked_by(c8, white)) {
              add_move(moves_list, encode_move(e8, c8, bK, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    if ((side_to_move == white) ? piece == wN : piece == bN) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = knight_attacks[src_sqr] & ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wB : piece == bB) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_bishop_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wR : piece == bR) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_rook_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wQ : piece == bQ) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_queen_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wK : piece == bK) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = king_attacks[src_sqr] & ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }
  }
}

static inline void generate_capture_moves(Moves *moves_list) {
  moves_list->count = 0;
  int src_sqr, dest_sqr;
  U64 position, attacks; // current iteration's piece bitboard & its attacks map

  int base = side_to_move == white? wP : bP;
  for (int i = 0; i < 6; i++) {
    int piece = base + i;
    position = bitboards[piece];

    // generating pawn capture moves
    if (side_to_move == white) {
      if (piece == wP) {
        while (position) {
          src_sqr = get_lsb_index(position);

          // Only pawn captures (no quiet moves)
          attacks = pawn_attacks[white][src_sqr] & sides_occupancies[black];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0) { /* shouldn't happen because while(attacks) guards it */ continue; }
            // pawn capture promotion move
            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 1, 0, 0, 0));
            }
            // regular pawn capture move
            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          // en passant capture
          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[white][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[bP], en_passant + 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, wP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
    } else {
      if (piece == bP) {
        while (position) {
          src_sqr = get_lsb_index(position);

          // Only pawn captures (no quiet moves)
          attacks = pawn_attacks[black][src_sqr] & sides_occupancies[white];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0) { /* shouldn't happen because while(attacks) guards it */ continue; }
            // pawn capture promotion move
            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 1, 0, 0, 0));
            }
            // regular pawn capture move
            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          // en passant capture
          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[black][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[wP], en_passant - 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, bP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
    }

    // knight captures only
    if ((side_to_move == white) ? piece == wN : piece == bN) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = knight_attacks[src_sqr] & sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    // bishop captures only
    if ((side_to_move == white) ? piece == wB : piece == bB) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_bishop_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    // rook captures only
    if ((side_to_move == white) ? piece == wR : piece == bR) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_rook_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    // queen captures only
    if ((side_to_move == white) ? piece == wQ : piece == bQ) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_queen_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    // king captures only (no castling)
    if ((side_to_move == white) ? piece == wK : piece == bK) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = king_attacks[src_sqr] & sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }
  }
}


/* **********************
1111 = qkQK = 15
wK    moved = 1111 & 1100 = 12
h1 wR moved = 1111 & 1110 = 14
a1 wR moved = 1111 & 1101 = 13

bK    moved = 1111 & 0011 = 3
h8 bR moved = 1111 & 1011 = 11
a8 bR moved = 1111 & 0111 = 7
************************* */

const int castling_rights[64] = {
    7,  15, 15, 15, 3,  15, 15, 11, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14,
};

#define COPY_BOARD()                                                           \
  U64 bitboards_copy[12], sides_occupancies_copy[3], hash_key_copy;            \
  int side_to_move_copy, en_passant_copy, can_castle_copy;                     \
  memcpy(bitboards_copy, bitboards, 96);                                       \
  memcpy(sides_occupancies_copy, sides_occupancies, 24);                       \
  side_to_move_copy = side_to_move, en_passant_copy = en_passant,              \
  can_castle_copy = can_castle;                                                \
  hash_key_copy = hash_key;


#define RESTORE_BOARD()                                                        \
  memcpy(bitboards, bitboards_copy, 96);                                       \
  memcpy(sides_occupancies, sides_occupancies_copy, 24);                       \
  side_to_move = side_to_move_copy, en_passant = en_passant_copy,              \
  can_castle = can_castle_copy;                                                \
  hash_key = hash_key_copy

enum { allow_all_moves, allow_only_captures };

// --- make move ---

static inline int make_move(int move, int move_flag) {
  // quiet moves
  if (move_flag == allow_all_moves) {
    COPY_BOARD();

    int source_sqr = get_move_source(move);
    int target_sqr = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted_piece(move);
    int capture_flag = get_move_capture_flag(move);
    int castling_flag = get_move_castling_flag(move);
    int double_push_flag = get_move_double_push_flag(move);
    int en_passant_flag = get_move_en_passant_flag(move);

    // --- FIX START: Validation Check ---
    // 1. Check bounds (you had this)
    if (source_sqr < 0 || source_sqr >= 64 ||
        target_sqr < 0 || target_sqr >= 64 ||
        piece < 0 || piece >= 12 || move == 0 || source_sqr == target_sqr) {
      RESTORE_BOARD();
      return 0;
    }

    // 2. CRITICAL: Check if the piece is actually there!
    // This stops the "Teleporting Pawn" bug caused by hash collisions.
    if (!get_bit(bitboards[piece], source_sqr)) {
        RESTORE_BOARD();
        return 0;
    }
    // --- FIX END ---

    // move piece
    pop_bit(bitboards[piece], source_sqr);
    set_bit(bitboards[piece], target_sqr);

    /* update piece location in the hash position */
    hash_key ^= piece_keys[piece][source_sqr]; // remove it from source
    hash_key ^= piece_keys[piece][target_sqr]; // add it to target

    if (capture_flag) {
      int start_piece = side_to_move == white ? bP : wP,
          end_piece = side_to_move == white ? bK : wK;

      for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
        if (get_bit(bitboards[bb_piece], target_sqr)) {
          pop_bit(bitboards[bb_piece], target_sqr);

          // update hash: remove piece from hash position
          hash_key ^= piece_keys[bb_piece][target_sqr];

          break;
        }
      }
    }

    if (promoted_piece) {
      pop_bit(bitboards[(side_to_move == white) ? wP : bP], target_sqr);

      set_bit(bitboards[promoted_piece], target_sqr);
      // remove hashed pawn preset earlier with the promoted piece
      hash_key ^= piece_keys[piece][target_sqr]; // remove pawn
      hash_key ^= piece_keys[promoted_piece][target_sqr]; // add promoted piece
    }
    if (en_passant_flag) { // en passant capture
      if (side_to_move == white) {
        pop_bit(bitboards[bP], target_sqr + 8); // remove captured en passant pawn
        hash_key ^= piece_keys[bP][target_sqr + 8]; // update hash
      } else {
        pop_bit(bitboards[wP], target_sqr - 8); // remove captured enpassant pawn
        hash_key ^= piece_keys[wP][target_sqr - 8]; // update hash
      }
    }

    // if captured pawn using en passant, remove the en passant hashed square
    if (en_passant != no_square) hash_key ^= enpassant_keys[en_passant];
    en_passant = no_square;

    if (double_push_flag) {
      en_passant = (side_to_move == white) ? target_sqr + 8 : target_sqr - 8;

      /* update hash: en passant square */
      hash_key ^= enpassant_keys[en_passant];
    }

    if (castling_flag) {
      switch (target_sqr) {
      // WCK
      case (g1):
        pop_bit(bitboards[wR], h1);
        set_bit(bitboards[wR], f1);

        hash_key ^= piece_keys[wR][h1];
        hash_key ^= piece_keys[wR][f1];  // update rook hash position on castling
        break;
      // WCQ
      case (c1):
        pop_bit(bitboards[wR], a1);
        set_bit(bitboards[wR], d1);


        hash_key ^= piece_keys[wR][a1];
        hash_key ^= piece_keys[wR][d1];  // update rook hash position on castling
        break;
      // BCK
      case (g8):
        pop_bit(bitboards[bR], h8);
        set_bit(bitboards[bR], f8);

        hash_key ^= piece_keys[bR][h8];
        hash_key ^= piece_keys[bR][f8];  // update rook hash position on castling
        break;

      // BCQ
      case (c8):
        pop_bit(bitboards[bR], a8);
        set_bit(bitboards[bR], d8);

        hash_key ^= piece_keys[bR][a8];
        hash_key ^= piece_keys[bR][d8];  // update rook hash position on castling
        break;
      }
    }

    // update hash: remove previous castling rights
    hash_key ^= castle_keys[can_castle];

    // update castling rights
    can_castle &= castling_rights[source_sqr];
    can_castle &= castling_rights[target_sqr];

    // update hash: add new hashed castling rights
    hash_key ^= castle_keys[can_castle];

    set_sides_occupancies();

    side_to_move ^= 1;
    // if side was white, now it is black therefore we hash it with the side key
    // if side was black, it was already hashed, now is white so ^= key to unhash it
    hash_key ^= side_to_move_key;

    // -------------------------------------- //
    // ---- HASH KEY TEST ---- //
    // -------------------------------------- //

    // U64 whole_hash_key = update_hash_key();
    // if (hash_key != whole_hash_key){
    //   print_board(1);
    //   printf("\033[1;93mMAKE MOVE\033[0;0m move: %s\n", get_move_str(move));
    //   printf("\033[1;36m%llx\033[0;0m should be %llx\n\n", hash_key, whole_hash_key);
    //
    // }


   int king_sq;
    if (side_to_move == white) {
      /* we just flipped side_to_move, so check black king */
      king_sq = get_lsb_index(bitboards[bK]);
    } else {
      king_sq = get_lsb_index(bitboards[wK]);
    }

    /* if king missing (bitboard unexpectedly zero), treat move as illegal */
    if (king_sq < 0) {
      /* Defensive: restore prior state and fail the move */
      RESTORE_BOARD();
      return 0;
    }

    /* only now call is_square_attacked_by */
    if (is_square_attacked_by(king_sq, side_to_move)) {
      RESTORE_BOARD();
      return 0; /* illegal move (king in check) */
    } else {
      return 1; /* legal */
    }  }

  // capture moves
  else {
    if (get_move_capture_flag(move)) {
      return make_move(move, allow_all_moves);
    } else {
      return 0;
    }
  }

  return 0;
}

/***** MAIN FUNCTION *****/

void init_default_board_position() {
  bitboards[bP] = 65280ULL;
  bitboards[wP] = 71776119061217280ULL;

  bitboards[wR] = 9295429630892703744ULL;
  bitboards[bR] = 129ULL;

  bitboards[wB] = 2594073385365405696ULL;
  bitboards[bB] = 36ULL;

  bitboards[wN] = 4755801206503243776ULL;
  bitboards[bN] = 66ULL;

  bitboards[wQ] = 576460752303423488ULL;
  bitboards[bQ] = 8ULL;

  bitboards[wK] = 1152921504606846976ULL;
  bitboards[bK] = 16ULL;

  sides_occupancies[black] = 65535ULL;
  sides_occupancies[white] = 18446462598732840960ULL;
  sides_occupancies[both] = 18446462598732906495ULL;
}

// PERFT

int get_time_ms() {
#if defined(_WIN64) || defined(_WIN32)
  return GetTickCount();
#else
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return (int)time_value.tv_sec * 1000 + (int)time_value.tv_usec / 1000;
#endif
}

U64 nodes; // positions generated by move gen

static inline void perft_driver(int depth) {
  if (depth == 0) {
    nodes++;
    return;
  }

  Moves ml;
  ml.count = 0;

  generate_moves(&ml);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    COPY_BOARD();

    if (!make_move(move, allow_all_moves)) {
      continue;
    }
    perft_driver(depth - 1); // call perft recursively

    RESTORE_BOARD();


    // U64 whole_hash_key = update_hash_key();
    // if (hash_key != whole_hash_key){
    //   printf("\033[1;93mPERFT DRIVER\033[0;0m move: %s\n", get_move_str(move));
    //   printf("\033[1;36m%llx\033[0;0m should be %llx\n\n", hash_key, whole_hash_key);
    //
    // }
  }
}

void perft_test(int depth) {

  printf("\n  \033[1;92mPerformance Test\033[0;0m\n\n");

  Moves ml;
  ml.count = 0;

  generate_moves(&ml);

  for (int i = 0; i < ml.count; i++) {
    COPY_BOARD();

    if (!make_move(ml.moves[i], allow_all_moves)) {
      continue;
    }

    U64 prev_nodes = nodes;


    perft_driver(depth - 1);

    U64 new_nodes = nodes - prev_nodes;

    RESTORE_BOARD();

    printf("    \033[1;93mmove: \033[0;0m");
    printf("%s-%s%c \033[1;93mnodes: \033[0;0m%llu\n",
           square_to_notation[get_move_source(ml.moves[i])],
           square_to_notation[get_move_target(ml.moves[i])],
           ascii_promoted_pieces[get_move_promoted_piece(ml.moves[i])],
           new_nodes);
  }
}

/**********************************\
 ==================================

       Time controls variables

 ==================================
\**********************************/

// exit from engine flag
int quit = 0;

int movestogo = 30;

int movetime = -1;

int time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;


int input_waiting()
{
    #ifndef WIN32
        fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
    #else
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init)
        {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe)
            {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }

        if (pipe)
        {
           if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
           return dw;
        }

        else
        {
           GetNumberOfConsoleInputEvents(inh, &dw);
           return dw <= 1 ? 0 : dw;
        }

    #endif
}

// read GUI/user input
void read_input()
{
    // bytes to read holder
    ssize_t bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting())
    {
        // tell engine to stop calculating
        stopped = 1;

        // loop to read bytes from STDIN
        do
        {
            // read bytes from STDIN
            bytes=read(fileno(stdin), input, 256);
        }

        // until bytes available
        while (bytes < 0);

        // searches for the first occurrence of '\n'
        endc = strchr(input,'\n');

        // if found new line set value at pointer to 0
        if (endc) *endc=0;

        // if input is available
        if (strlen(input) > 0)
        {
            // match UCI "quit" command
            if (strncmp(input, "quit", 4) == 0)
            {
                // tell engine to terminate exacution
                quit = 1;
            }

            // // match UCI "stop" command
            else if (strncmp(input, "stop", 4) == 0)    {
                // tell engine to terminate exacution
                quit = 1;
            }
        }
    }
}

// a bridge function to interact between search and GUI input
static void communicate() {
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptime) {
		// tell engine to stop calculating
		stopped = 1;
	}

    // read GUI input
	read_input();
}

const int pst_score[12][64] = {
  // White Pawn
  {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0
  },

  // White Knight
  {
    -167, -89, -34, -49,  61, -97, -15,-107,
     -73, -41,  72,  36,  23,  62,   7, -17,
     -47,  60,  37,  65,  84, 129,  73,  44,
      -9,  17,  19,  53,  37,  69,  18,  22,
     -13,   4,  16,  13,  28,  19,  21,  -8,
     -23,  -9,  12,  10,  19,  17,  25, -16,
     -29, -53, -12,  -3,  -1,  18, -14, -19,
    -105, -21, -58, -33, -17, -28, -19, -23
  },

  // White Bishop
  {
    -29,4,-82,-37,-25,-42,7,-8,
    -26,16,-18,-13,30,59,18,-47,
    -16,37,43,40,35,50,37,-2,
    -4,5,19,50,37,37,7,-2,
    -6,13,13,26,34,12,10,4,
    0,15,15,15,14,27,18,10,
    4,15,16,0,7,21,33,1,
    -33,-3,-14,-21,-13,-12,-39,-21
  },

  // White Rook
  {
    32,42,32,51,63,9,31,43,
    27,32,58,62,80,67,26,44,
    -5,19,26,36,17,45,61,16,
    -24,-11,7,26,24,35,-8,-20,
    -36,-26,-12,-1,9,-7,6,-23,
    -45,-25,-16,-17,3,0,-5,-33,
    -44,-16,-20,-9,-1,11,-6,-71,
    -19,-13,1,17,16,7,-37,-26
  },

  // White Queen
  {
    -28,0,29,12,59,44,43,45,
    -24,-39,-5,1,-16,57,28,54,
    -13,-17,7,8,29,56,47,57,
    -27,-27,-16,-16,-1,17,-2,1,
    -9,-26,-9,-10,-2,-4,3,-3,
    -14,2,-11,-2,-5,2,14,5,
    -35,-8,11,2,8,15,-3,1,
    -1,-18,-9,10,-15,-25,-31,-50
  },

  // White King (MG)
  {
    -65,23,16,-15,-56,-34,2,13,
    29,-1,-20,-7,-8,-4,-38,-29,
    -9,24,2,-16,-20,6,22,-22,
    -17,-20,-12,-27,-30,-25,-14,-36,
    -49,-1,-27,-39,-46,-44,-33,-51,
    -14,-14,-22,-46,-44,-30,-15,-27,
    1,7,-8,-64,-43,-16,9,8,
    -15,36,12,-54,8,-28,24,14
  },

  // Black Pawn (negated)
  {
    0,0,0,0,0,0,0,0,
    35,1,20,23,15,-24,-38,22,
    26,4,4,10,-3,-3,-33,12,
    27,2,5,-12,-17,-6,-10,25,
    14,-13,-6,-21,-23,-12,-17,23,
    6,-7,-26,-31,-65,-56,-25,20,
    -98,-134,-61,-95,-68,-126,-34,11,
    0,0,0,0,0,0,0,0
  },

  // Black Knight (negated)
  {
    105,21,58,33,17,28,19,23,
    29,53,12,3,1,-18,14,19,
    23,9,-12,-10,-19,-17,-25,16,
    13,-4,-16,-13,-28,-19,-21,8,
    9,-17,-19,-53,-37,-69,-18,-22,
    47,-60,-37,-65,-84,-129,-73,-44,
    73,41,-72,-36,-23,-62,-7,17,
    167,89,34,49,-61,97,15,107
  },

  // Black Bishop (negated)
  {
    33,3,14,21,13,12,39,21,
    -4,-15,-16,0,-7,-21,-33,-1,
    0,-15,-15,-15,-14,-27,-18,-10,
    6,-13,-13,-26,-34,-12,-10,-4,
    4,-5,-19,-50,-37,-37,-7,2,
    16,-37,-43,-40,-35,-50,-37,2,
    26,-16,18,13,-30,-59,-18,47,
    29,-4,82,37,25,42,-7,8
  },

  // Black Rook (negated)
  {
    19,13,-1,-17,-16,-7,37,26,
    44,16,20,9,1,-11,6,71,
    45,25,16,17,-3,0,5,33,
    36,26,12,1,-9,7,-6,23,
    24,11,-7,-26,-24,-35,8,20,
    5,-19,-26,-36,-17,-45,-61,-16,
    -27,-32,-58,-62,-80,-67,-26,-44,
    -32,-42,-32,-51,-63,-9,-31,-43
  },

  // Black Queen (negated)
  {
    1,18,9,-10,15,25,31,50,
    35,8,-11,-2,-8,-15,3,-1,
    14,-2,11,2,5,-2,-14,-5,
    9,26,9,10,2,4,-3,3,
    27,27,16,16,1,-17,2,-1,
    13,17,-7,-8,-29,-56,-47,-57,
    24,39,5,-1,16,-57,-28,-54,
    28,0,-29,-12,-59,-44,-43,-45
  },

  // Black King (MG negated)
  {
    15,-36,-12,54,-8,28,-24,-14,
    -1,-7,8,64,43,16,-9,-8,
    14,14,22,46,44,30,15,27,
    49,1,27,39,46,44,33,51,
    17,20,12,27,30,25,14,36,
    9,-24,-2,16,20,-6,-22,22,
    -29,1,20,7,8,4,38,29,
    65,-23,-16,15,56,34,-2,-13
  }
};

// Updated material values (more standard)
int material_score[12] = {
  100,   // wP
  320,   // wN
  330,   // wB
  500,   // wR
  900,   // wQ
  20000, // wK
  -100,  // bP
  -320,  // bN
  -330,  // bB
  -500,  // bR
  -900,  // bQ
  -20000 // bK
};

// Most Valuable Victim (MVV) - Least Valuable Attacker lookup table (LVA)
// note: (might look redundant but the use of a 12x12
// is for en_passant's target_piece (defaults to wP, same result -> 105 score)
static int mvv_lva[12][12] = {
  105, 205, 305, 405, 505, 605,   105, 205, 305, 405, 505, 605,
  104, 204, 304, 404, 504, 604,   104, 204, 304, 404, 504, 604,
  103, 203, 303, 403, 503, 603,   103, 203, 303, 403, 503, 603,
  102, 202, 302, 402, 502, 602,   102, 202, 302, 402, 502, 602,
  101, 201, 301, 401, 501, 601,   101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600,   100, 200, 300, 400, 500, 600,

  105, 205, 305, 405, 505, 605,   105, 205, 305, 405, 505, 605,
  104, 204, 304, 404, 504, 604,   104, 204, 304, 404, 504, 604,
  103, 203, 303, 403, 503, 603,   103, 203, 303, 403, 503, 603,
  102, 202, 302, 402, 502, 602,   102, 202, 302, 402, 502, 602,
  101, 201, 301, 401, 501, 601,   101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600,   100, 200, 300, 400, 500, 600,
};

// bonus for pushing enemy king closer to the edges (for checkmating)
// (center manhattan distance)
const int cmd_score[64] = {
    200, 150, 100,  50,  50, 100, 150, 200,
    150, 100,  50,  20,  20,  50, 100, 150,
    100,  50,  20,  10,  10,  20,  50, 100,
     50,  20,  10,   0,   0,  10,  20,  50,
     50,  20,  10,   0,   0,  10,  20,  50,
    100,  50,  20,  10,  10,  20,  50, 100,
    150, 100,  50,  20,  20,  50, 100, 150,
    200, 150, 100,  50,  50, 100, 150, 200
};

static inline int eval() {
  int score = 0;
  U64 cur_bb;
  int square;

  // ==================================================================
  // 1. MATERIAL + PST FOR NON-PAWN, NON-SPECIAL PIECES (N, B, R, Q, K)
  // ==================================================================
  // Note: Pawns, Rooks, Bishops, Queens, and Kings are handled separately
  // below to include additional positional logic

  for (int piece = wP; piece <= bK; piece++) {
    // Skip pieces that have special handling below
    if (piece == wP || piece == bP) continue;  // Pawns: handled in section 3
    if (piece == wR || piece == bR) continue;  // Rooks: open/semi-open file logic
    if (piece == wK || piece == bK) continue;  // Kings: king safety logic
    if (piece == wB || piece == bB) continue;  // Bishops: mobility bonus
    if (piece == wQ || piece == bQ) continue;  // Queens: mobility bonus

    cur_bb = bitboards[piece];

    while (cur_bb) {
      square = get_lsb_index(cur_bb);
      int material = material_score[piece];
      int pst = pst_score[piece][square];
      score += material + pst;

      pop_bit(cur_bb, square);
    }
  }

  // ==================================================================
  // 2. MOP-UP ENDGAME (King distance in winning positions)
  // ==================================================================
  int white_king_sq = get_lsb_index(bitboards[wK]);
  int black_king_sq = get_lsb_index(bitboards[bK]);

  int white_mat =
    count_bits(bitboards[wP]) * material_score[wP] +
    count_bits(bitboards[wN]) * material_score[wN] +
    count_bits(bitboards[wB]) * material_score[wB] +
    count_bits(bitboards[wR]) * material_score[wR] +
    count_bits(bitboards[wQ]) * material_score[wQ];

  int black_mat =
    count_bits(bitboards[bP]) * -material_score[bP] +
    count_bits(bitboards[bN]) * -material_score[bN] +
    count_bits(bitboards[bB]) * -material_score[bB] +
    count_bits(bitboards[bR]) * -material_score[bR] +
    count_bits(bitboards[bQ]) * -material_score[bQ];

  int material_diff = white_mat - black_mat;

  int dist = abs(get_rank[white_king_sq] - get_rank[black_king_sq]) +
    abs(get_file(white_king_sq) - get_file(black_king_sq));

  if (material_diff > 300) {
    score += cmd_score[black_king_sq];

    int scale_factor = material_diff / 200;
    if (scale_factor > 4) scale_factor = 4;
    int kd_bonus = (14 - dist) * scale_factor;
    score += kd_bonus;
  }
  else if (material_diff < -300) {
    score -= cmd_score[white_king_sq];

    int scale_factor = -material_diff / 200;
    if (scale_factor > 4) scale_factor = 4;

    int kd_bonus = (14 - dist) * scale_factor;
    score -= kd_bonus;
  }

  // ==================================================================
  // 3. PAWN STRUCTURE (Passed, Isolated, Doubled) + Material + PST
  // ==================================================================
  int wP_file_count[8] = {0};
  int bP_file_count[8] = {0};

  U64 pawn_bb = bitboards[wP];

  while (pawn_bb) {
    square = get_lsb_index(pawn_bb);
    int r = get_rank[square];
    int f = get_file(square);

    wP_file_count[f]++;

    // Passed Pawn
    if (!(passed_pawns_mask[white][square] & bitboards[bP])) {
      int bonus = passed_pawn_bonus[r];
      score += bonus;
    }

    // Isolated Pawn
    if (!(isolated_pawns_mask[square] & bitboards[wP])) {
      score += isolated_pawn_penalty;
    }

    // Material + PST for white pawns
    int material = material_score[wP];
    int pst = pst_score[wP][square];
    score += material + pst;

    pop_bit(pawn_bb, square);
  }

  pawn_bb = bitboards[bP];

  while (pawn_bb) {
    square = get_lsb_index(pawn_bb);
    int r = get_rank[square];
    int f = get_file(square);

    bP_file_count[f]++;

    // Passed Pawn
    if (!(passed_pawns_mask[black][square] & bitboards[wP])) {
      int bonus = passed_pawn_bonus[7 - r];
      score -= bonus;
    }

    // Isolated Pawn
    if (!(isolated_pawns_mask[square] & bitboards[bP])) {
      score -= isolated_pawn_penalty;
    }

    // Material + PST for black pawns
    int material = material_score[bP];
    int pst = pst_score[bP][square];
    score += material + pst;

    pop_bit(pawn_bb, square);
  }

  // Doubled Pawns penalty
  for (int i = 0; i < 8; i++) {
    if (wP_file_count[i] > 1) {
      int p = double_pawn_penalty * (wP_file_count[i] - 1);
      score += p;
    }

    if (bP_file_count[i] > 1) {
      int p = double_pawn_penalty * (bP_file_count[i] - 1);
      score -= p;
    }
  }

  // ==================================================================
  // 4. ROOK EVALUATION (Open/Semi-open files + Material + PST)
  // ==================================================================
  U64 of_bb = bitboards[wR];
  while(of_bb) {
    square = get_lsb_index(of_bb);

    // Open file bonus
    if (((bitboards[wP] | bitboards[bP]) & pawns_file_mask[square]) == 0) {
      score += RookOpenFileBonus;
    }
    // Semi-open file bonus
    else if ((bitboards[wP] & pawns_file_mask[square]) == 0) {
      score += RookSemiOpenFileBonus;
    }

    int material = material_score[wR];
    int pst = pst_score[wR][square];
    score += material + pst;

    pop_bit(of_bb, square);
  }

  of_bb = bitboards[bR];
  while(of_bb) {
    square = get_lsb_index(of_bb);

    // Open file bonus
    if (((bitboards[wP] | bitboards[bP]) & pawns_file_mask[square]) == 0) {
      score -= RookOpenFileBonus;
    }
    // Semi-open file bonus
    else if ((bitboards[bP] & pawns_file_mask[square]) == 0) {
      score -= RookSemiOpenFileBonus;
    }

    int material = material_score[bR];
    int pst = pst_score[bR][square];
    score += material + pst;

    pop_bit(of_bb, square);
  }

  // ==================================================================
  // 5. KING SAFETY + Material + PST
  // ==================================================================
  of_bb = bitboards[wK];
  while(of_bb) {
    square = get_lsb_index(of_bb);

    // Unshielded king penalty (open file in front of king)
    if (((bitboards[wP] | bitboards[bP]) & pawns_file_mask[square]) == 0) {
      score -= UnShieldedKingPenalty;
    }
    // Semi-shielded (no friendly pawn on king's file)
    else if ((bitboards[wP] & pawns_file_mask[square]) == 0) {
      score -= SemiShieldedKingPenalty;
    }

    int material = material_score[wK];
    int pst = pst_score[wK][square];
    score += material + pst;

    // Pawn shield bonus
    score += count_bits(king_attacks[square] & bitboards[wP]) * KingShieldBonus;

    // Mobile king penalty (exposed king)
    score -= count_bits(get_queen_attacks(square, sides_occupancies[both]));

    pop_bit(of_bb, square);
  }

  of_bb = bitboards[bK];
  while(of_bb) {
    square = get_lsb_index(of_bb);

    // Unshielded king penalty
    if (((bitboards[wP] | bitboards[bP]) & pawns_file_mask[square]) == 0) {
      score += UnShieldedKingPenalty;
    }
    // Semi-shielded
    else if ((bitboards[bP] & pawns_file_mask[square]) == 0) {
      score += SemiShieldedKingPenalty;
    }

    int material = material_score[bK];
    int pst = pst_score[bK][square];
    score += material + pst;

    // Pawn shield bonus
    score -= count_bits(king_attacks[square] & bitboards[bP]) * KingShieldBonus;

    // Mobile king penalty
    score += count_bits(get_queen_attacks(square, sides_occupancies[both]));

    pop_bit(of_bb, square);
  }

  // ==================================================================
  // 6. BISHOP MOBILITY + Material + PST
  // ==================================================================
  U64 mob_bb = bitboards[wB];
  while(mob_bb) {
    square = get_lsb_index(mob_bb);

    int material = material_score[wB];
    int pst = pst_score[wB][square];
    score += material + pst;

    // Mobility bonus
    score += count_bits(get_bishop_attacks(square, sides_occupancies[both]));

    pop_bit(mob_bb, square);
  }

  mob_bb = bitboards[bB];
  while(mob_bb) {
    square = get_lsb_index(mob_bb);

    int material = material_score[bB];
    int pst = pst_score[bB][square];
    score += material + pst;

    // Mobility bonus
    score -= count_bits(get_bishop_attacks(square, sides_occupancies[both]));

    pop_bit(mob_bb, square);
  }

  // ==================================================================
  // 7. QUEEN MOBILITY + Material + PST
  // ==================================================================
  mob_bb = bitboards[wQ];
  while(mob_bb) {
    square = get_lsb_index(mob_bb);

    int material = material_score[wQ];
    int pst = pst_score[wQ][square];
    score += material + pst;

    // Mobility bonus
    score += count_bits(get_queen_attacks(square, sides_occupancies[both]));

    pop_bit(mob_bb, square);
  }

  mob_bb = bitboards[bQ];
  while(mob_bb) {
    square = get_lsb_index(mob_bb);

    int material = material_score[bQ];
    int pst = pst_score[bQ][square];
    score += material + pst;

    // Mobility bonus
    score -= count_bits(get_queen_attacks(square, sides_occupancies[both]));

    pop_bit(mob_bb, square);
  }

  // ==================================================================
  // 8. RETURN SCORE FROM SIDE-TO-MOVE PERSPECTIVE
  // ==================================================================
  int final_score = (side_to_move == white ? score : -score);
  return final_score;
}



#define MAX_PLY 128
#define MATE_VALUE 49000
#define MATE_SCORE 48000

int ply;  // half-move counter

int killer_moves[2][MAX_PLY]; // [side][ply]
int history_moves[12][64]; // [piece][square]

int pv_length[MAX_PLY];
int pv_table[MAX_PLY][MAX_PLY];

int apply_pv, pv_score;

// ----------------------------- //
// ---- TRANSPOSITION TABLE ---- //
// ----------------------------- //

#define TT_SIZE_MB 64
#define TT_SIZE_BYTES (TT_SIZE_MB * 1024 * 1024)

#define hashf_EXACT 0
#define hashf_ALPHA 1 // Upper Bound (We know score <= alpha) - Fail Low
#define hashf_BETA  2 // Lower Bound (We know score >= beta)  - Fail High

// 100,000 to ensure it goes outside the bound of alpha-beta which could be the return value aswell
#define NO_TT_ENTRY_FOUND 100000

typedef struct {
  U64         key; // uncompressed, might compress to 16bits later to save memory
  int8_t    depth; // 255: score and move calculated at this depth
  int8_t     flag; // EXACT, LOWERBOUND, UPPERBOUND
  int16_t   score; // 65535: alpha/beta/PV
  int       move; // compress: SSSS SSTT TTTT PPPP (src/dest/promo)
} TT_Entry; // size: 16 bytes / entry (uncompressed)

size_t tt_size = TT_SIZE_BYTES / sizeof(TT_Entry); // max num entries
int16_t age = 0; // search generation (incremented each search cycle)
TT_Entry* TranspositionTable;

void clear_tt() { memset(TranspositionTable, 0, TT_SIZE_BYTES); }

void init_tt() {
  TranspositionTable = calloc(tt_size, sizeof(TT_Entry)); // calloc = malloc + memset
  if(!TranspositionTable) printf("ERROR! couldn't initialize transposition table.\n");
}

// Standardized Mate Score handling
static inline int probeTT(int alpha, int beta, int depth) {
  size_t index = hash_key & (tt_size - 1);
  TT_Entry* tt_entry = &TranspositionTable[index];

  if (tt_entry->key == hash_key) {
    if (tt_entry->depth >= depth) {
      int score = tt_entry->score;

      // Re-adjust mate score to be relative to current ply
      if (score > MATE_SCORE) score -= ply;
      if (score < -MATE_SCORE) score += ply;

      if (tt_entry->flag == hashf_EXACT) return score;
      if (tt_entry->flag == hashf_ALPHA && score <= alpha) return alpha;
      if (tt_entry->flag == hashf_BETA && score >= beta) return beta;
    }
  }
  return NO_TT_ENTRY_FOUND;
}

int probe_move(void) {
    TT_Entry* tt_entry = &TranspositionTable[hash_key & (tt_size - 1)];
    if (tt_entry->key == hash_key) {
        return tt_entry->move;
    }
    return 0;
}

void storeTT(int score, int depth, int hashf, int move) {
  size_t index = hash_key & (tt_size - 1);
  TT_Entry* tt_entry = &TranspositionTable[index];

  // Store mate score relative to root (independent of current ply)
  if (score > MATE_SCORE) score += ply;
  if (score < -MATE_SCORE) score -= ply;

  // Always replace if new entry is deeper, OR if it's an exact match (update move/score)
  // OR if the current entry is from an old position (collision resolution strategy)
  if (tt_entry->key == 0 || depth >= tt_entry->depth || tt_entry->key != hash_key) {
    tt_entry->key = hash_key;
    tt_entry->depth = (int8_t)depth;
    tt_entry->score = (int16_t)score;
    tt_entry->flag = (int8_t)hashf;

    // This prevents overwriting a valuable hash move with '0' (null) during a Fail-Low.
    if (move != 0) tt_entry->move = move;
  }
}

static inline void enable_pv_scoring(Moves* ml) {
  apply_pv = 0; // reset pv detection flag

  for(int i = 0; i < ml -> count; i++) {
    if(pv_table[0][ply] == ml -> moves[i]) {
      pv_score = 1;

      apply_pv = 1;
    }
  }
}

static inline int score_move(int move, int tt_move) {

  if(move == tt_move) return 30000;

  if (pv_score) { // if pv line can be applied
    if (pv_table[0][ply] == move) { // check for pv match
      pv_score = 0; // found the pv, stop searching
      return 20000; // return highest score
    }
  }

  if(get_move_capture_flag(move)) {
    int target_piece = wP;  // Default for en passant

    if (get_move_en_passant_flag(move)) {
      // En passant always captures a pawn
      target_piece = (side_to_move == white) ? bP : wP;
    } else {
      int start_piece = side_to_move == white ? bP : wP;
      int end_piece = side_to_move == white ? bK : wK;

      for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
        if (get_bit(bitboards[bb_piece], get_move_target(move))) {
          target_piece = bb_piece;
          break;
        }
      }
    }

    return mvv_lva[get_move_piece(move)][target_piece] + 10000;
  }
  else { // killer, quiet
    if (killer_moves[0][ply] == move) return 9000;
    else if (killer_moves[1][ply] == move) return 8000;
    else return history_moves[get_move_piece(move)][get_move_target(move)]; // default: 0 else:depth^2

  }

  return 0;
}


static inline void sort_moves(Moves *ml) {
    if (!ml || ml->count <= 1) return;

    int scores[MOVES_CAPACITY];
    int tt_move = probe_move();

    // Score all moves once
    for (int i = 0; i < ml->count; i++) {
        scores[i] = score_move(ml->moves[i], tt_move);
    }

    // Insertion Sort (Faster for small N)
    for (int i = 1; i < ml->count; i++) {
        int key_score = scores[i];
        int key_move = ml->moves[i];
        int j = i - 1;

        while (j >= 0 && scores[j] < key_score) {
            scores[j + 1] = scores[j];
            ml->moves[j + 1] = ml->moves[j];
            j--;
        }
        scores[j + 1] = key_score;
        ml->moves[j + 1] = key_move;
    }
}


static inline int quiescence_search(int alpha, int beta, int qs_depth) {

  // Check limits every 2047 nodes
  if ((nodes & 2047) == 0)
    communicate();

  nodes++;

  int eval_score = eval();

  if (ply >= MAX_PLY - 1)
    return eval_score;


  // Max QS depth to prevent search explosion
  if (qs_depth <= -10) {
    return eval_score;
  }

  // Beta cutoff
  if (eval_score >= beta)
    return beta;

  // Update alpha
  if (eval_score > alpha)
    alpha = eval_score;

  // Generate and sort capture moves
  Moves ml;
  generate_capture_moves(&ml);
  sort_moves(&ml);

  for (int i = 0; i < ml.count; i++) {
    // Delta pruning on individual moves
    int target_piece = wP;
    if (!get_move_en_passant_flag(ml.moves[i])) {
      int start = (side_to_move == white) ? bP : wP;
      int end = (side_to_move == white) ? bK : wK;

      for (int bb_piece = start; bb_piece <= end; bb_piece++) {
        if (get_bit(bitboards[bb_piece], get_move_target(ml.moves[i]))) {
          target_piece = bb_piece;
          break;
        }
      }
    } else {
      target_piece = (side_to_move == white) ? bP : wP;
    }

    int capture_value = abs(material_score[target_piece]);

    // If capturing this piece still can't raise alpha, skip it
    if (eval_score + capture_value + 200 < alpha)
      continue;

    COPY_BOARD();

    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (make_move(ml.moves[i], allow_only_captures) == 0) {
      ply--;
      repetition_index--;
      continue;
    }

    int score = -quiescence_search(-beta, -alpha, qs_depth - 1);

    ply--;
    repetition_index--;

    RESTORE_BOARD();

    if (stopped) return 0;

    if (score >= beta)
      return beta;

    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

// Enhanced Negamax with improved LMR and extensions
static inline int negamax(int alpha, int beta, int depth) {

  // 1. Check for Repetition / 50-move rule
  if(ply && is_repetition()) return 0;

  // 2. Probe Transposition Table
  int hashf_flag = hashf_ALPHA;
  int pv_node = (beta - alpha) > 1;
  int tt_bestmove = 0;
  int val;

  if (ply && !pv_node && ((val = probeTT(alpha, beta, depth)) != NO_TT_ENTRY_FOUND)) {
    return val;
  }

  // 3. Check for GUI input
  if ((nodes & 2047) == 0) communicate();
  if (stopped == 1) return alpha;

  pv_length[ply] = ply;

  // NOW we check if we hit the horizon.
  if (depth <= 0) {
    return quiescence_search(alpha, beta, 0);
  }

  if (ply >= MAX_PLY - 1) return eval();

  nodes++;

  // 5. MAX PLY Guard

  // =============================================================
  // CRITICAL FIX START: Calculate In-Check and Extend BEFORE Q-Search
  // =============================================================

  // Determine if we are in check
  int king_sq, enemy_king_sq;
  if (side_to_move == white) {
      king_sq = get_lsb_index(bitboards[wK]);
      enemy_king_sq = get_lsb_index(bitboards[bK]);
  } else {
      king_sq = get_lsb_index(bitboards[bK]);
      enemy_king_sq = get_lsb_index(bitboards[wK]);
  }

  int in_check = is_square_attacked_by(king_sq, side_to_move ^ 1);

  // CHECK EXTENSION: If in check, extend depth to ensure we find an evasion
  if (in_check) depth++;


  // NULL MOVE PRUNING
  if (depth >= 4 && !in_check && ply) {
    COPY_BOARD();
    ply ++;

    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if(en_passant != no_square) hash_key ^= enpassant_keys[en_passant];
    en_passant = no_square;

    side_to_move ^= 1;
    hash_key ^= side_to_move_key;

    int score = -negamax(-beta, -beta + 1, depth - 3);

    ply--; repetition_index--;
    RESTORE_BOARD();

    if (stopped) return 0;

    if (score >= beta)
      return beta;
  }

  Moves ml[1];
  generate_moves(ml);

  if (apply_pv) {
    enable_pv_scoring(ml);
  }

  sort_moves(ml);

  int moves_searched = 0;
  int found_pv = 0;

  int legal_moves = 0;

  for (int i = 0; i < ml->count; i++) {
    COPY_BOARD();
    ply++;

    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (make_move(ml->moves[i], allow_all_moves) == 0) {
      ply--; repetition_index--;
      continue;
    }

    legal_moves++;

    int move_depth = depth;
    int extension  = 0;

    /* Precompute any data that doesn't depend on the move */
    const int can_extend   = (ply < MAX_PLY - 1);

    // We check if the opponent's king is attacked by US
    int gives_check = is_square_attacked_by(enemy_king_sq, side_to_move ^ 1);

    /* Promotion flag */
    const int is_promotion = get_move_promoted_piece(ml->moves[i]) != 0;
    const int is_capture = get_move_capture_flag(ml->moves[i]);

    /* === SEARCH EXTENSION === */

    if (can_extend && is_promotion) extension = 1;
    else if (can_extend) {
      if (is_capture && ply > 0) {

        /* Extract previous move once */
        const int prev = pv_table[ply - 1][ply - 1];

        /* Previous move must also be a capture and same target */
        if ( prev &&
          get_move_capture_flag(prev) &&
          get_move_target(prev) == get_move_target(ml -> moves[i]) )
        {
          extension = 1;
        }
      }
    }

    move_depth += extension;
    int score;
    int do_full_search = 0;
    int piece = get_move_piece(ml->moves[i]);
    int target = get_move_target(ml->moves[i]);
    int hist_score = history_moves[piece][target];

    // High history threshold - moves with good history shouldn't be reduced
    // Use depth squared as threshold since history scores are incremented by depth^2
    int history_threshold = depth * depth * 4;

    // === LATE MOVE REDUCTION (LMR) ===

    // Conditions for LMR:
    // - Not the first few moves (move index >= 3)
    // - Not a capture or promotion (quiet move)
    // - Not in check and doesn't give check
    // - Sufficient depth remaining (depth >= 3)
    // - Not a killer move
    // - Not a high-history move (moves that have been good in the past)

    int can_reduce = (moves_searched >= 4 &&          // After first 3 moves
                      depth >= 3 &&                    // Sufficient depth
                      !is_capture &&                   // Not a capture
                      !is_promotion &&                 // Not a promotion
                      !in_check &&                     // Not in check
                      !gives_check &&                  // Doesn't give check
                      hist_score < history_threshold); // Not high-history move
    if (can_reduce) {
      // Calculate reduction based on depth and move number
      int reduction = 1 + (depth / 3) + (moves_searched / 10);

      if (reduction > depth - 1)
        reduction = depth - 1;
      if (reduction < 1)
        reduction = 1;

      int reduced_depth = move_depth - reduction - 1;
      if (reduced_depth < 1)
        reduced_depth = 1;

      // Search with reduced depth and null window
      score = -negamax(-alpha - 1, -alpha, reduced_depth);

      // If reduced search fails high, need full depth search
      do_full_search = (score > alpha);
    }
    // Principal Variation Search (PVS) - null window for non-PV nodes
    else if (found_pv) {
      score = -negamax(-alpha - 1, -alpha, move_depth - 1);
      do_full_search = (score > alpha && score < beta);
    }
    // First move - always full window
    else {
      do_full_search = 1;
    }

    // Do full depth, full window search if needed
    if (do_full_search) {
      score = -negamax(-beta, -alpha, move_depth - 1);
    }

    moves_searched++;
    ply--; repetition_index--;
    RESTORE_BOARD();

    if (stopped) return 0;

    // Beta cutoff
    if (score >= beta) {

      storeTT(beta, depth, hashf_BETA, ml -> moves[i]);

      // Store killer moves (non-captures only)
      if (!is_capture && !is_promotion) {
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = ml->moves[i];
      }
      return beta;
    }

    // Alpha improvement (new best move found)
    if (score > alpha) {
      alpha = score;
      tt_bestmove = ml -> moves[i];
      found_pv = 1;
      hashf_flag = hashf_EXACT;

      // Update history heuristic (quiet moves only)
      if (!is_capture && !is_promotion) {
        history_moves[get_move_piece(ml->moves[i])][get_move_target(ml->moves[i])] += depth * depth;
      }

      // Update PV table
      pv_table[ply][ply] = ml->moves[i];

      for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
      }

      pv_length[ply] = pv_length[ply + 1];
    }
  }

  // No legal moves - checkmate or stalemate
  if (legal_moves == 0) {
    if (in_check)
      return ply - MATE_VALUE; // Checkmate (prefer faster mates)
    else
      return 0; // Stalemate
  }

  storeTT(alpha, depth, hashf_flag, tt_bestmove);
  return alpha;
}

// Enhanced search with aspiration windows
void search_position(int depth) {
  // 1. CRITICAL: Reset ply
  ply = 0;

  // Reset other stats
  nodes = 0;
  stopped = 0;
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_moves, 0, sizeof(history_moves));
  memset(pv_table, 0, sizeof(pv_table));
  memset(pv_length, 0, sizeof(pv_length));

  // Clear TT on new game (optional but recommended for debugging)
  // clear_tt();

  int best_move = 0;

  // Iterative Deepening
  for (int cur_depth = 1; cur_depth <= depth; cur_depth++) {
    if (stopped == 1) break;

    // Aspiration Window Logic (Simplified for stability)
    int score = negamax(NEG_INF, INF, cur_depth);

    if (stopped == 1) break;

    // Update best move from PV table
    if (pv_length[0] > 0) best_move = pv_table[0][0];

    // --- FIX: Correct Mate Score Printing ---
    printf("info depth %d score ", cur_depth);

    if (score > MATE_SCORE) {
      // Mate for us: (MATE_VALUE - score + 1) / 2
      printf("mate %d ", (MATE_VALUE - score + 1) / 2);
    }
    else if (score < -MATE_SCORE) {
      // Mate against us: -(score + MATE_VALUE) / 2
      printf("mate %d ", -(score + MATE_VALUE) / 2);
    }
    else {
      printf("cp %d ", score);
    }

    printf("nodes %llu pv ", nodes);
    for (int i = 0; i < pv_length[0]; i++) {
      printf("%s ", get_move_str(pv_table[0][i]));
    }
    printf("\n");
  }

  printf("bestmove ");
  if (best_move) print_move(best_move);
  else {
    // Fallback if search failed to return a move (rare)
    Moves ml; ml.count = 0; generate_moves(&ml);
    if (ml.count > 0) print_move(ml.moves[0]);
    else print_move(0); // Resign/Mate
  }
}


int parse_move(char *move_str) { // move_str: e2e4, e7e8q, etc.
  Moves ml;
  ml.count = 0;
  generate_moves(&ml);

  if (strlen(move_str) < 4) return 0;

  // safety check
  if (move_str[0] < 'a' || move_str[0] > 'h') return 0;
  if (move_str[2] < 'a' || move_str[2] > 'h') return 0;

  int src_sqr  = (move_str[0] - 'a') + ((8 - (move_str[1] - '0')) * 8);
  int dest_sqr = (move_str[2] - 'a') + ((8 - (move_str[3] - '0')) * 8);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    if (get_move_source(move) == src_sqr &&
        get_move_target(move) == dest_sqr) {

      int pp = get_move_promoted_piece(move);

      /* non-promotion move */
      if (pp == 0 && (move_str[4] == '\0' || isspace((unsigned char)move_str[4]))) {
        return move;
      }

      /* promotion moves (accept lower/upper case), accept both white & black promo constants */
      if (move_str[4] != '\0') {
        char promo = (char)tolower((unsigned char)move_str[4]);

        if ((pp == wQ || pp == bQ) && promo == 'q') return move;
        if ((pp == wR || pp == bR) && promo == 'r') return move;
        if ((pp == wN || pp == bN) && promo == 'n') return move;
        if ((pp == wB || pp == bB) && promo == 'b') return move;
      }
    }
  }

  return 0; // illegal / not found
}


/*  position startpos
 *  position startpos moves e2e4 e7e5
 *  position startpos fen 8/8/8/8/8/8/8/8/8 w - - moves e2e4
 *  */
void parse_position(char *command) {
  command += 9; // skip "position "
  char *cur_char = command;

  // parse "startpos" cmd
  if(strncmp(command, "startpos", 8) == 0) {
    cur_char += 8;
    parse_fen(start_position);
  } else {
    cur_char = strstr(command, "fen");
    if (!cur_char) { parse_fen(start_position); }
    else {
      cur_char += 4;
      parse_fen(cur_char);
    }
  }

  cur_char = strstr(cur_char, "moves");
  if(cur_char) {
    cur_char += 6;

    while(*cur_char) {
      int move = parse_move(cur_char);
      if (!move) { break; }

      repetition_index++;
      repetition_table[repetition_index] = hash_key;

      make_move(move, allow_all_moves);
      while(*cur_char && *cur_char != ' ') {cur_char++;}
      cur_char++;
    }
  }
}

void parse_uci_makemoves(char *command) {
  command += 10;
  char *cur_char = command;

  while (*cur_char) {
    int move = parse_move(cur_char);
    if (!move) { break; }
    make_move(move, allow_all_moves);
    while(*cur_char && *cur_char != ' ') {cur_char++;}
    cur_char++;
  }
}

/*
 * go
 * go moves e2e4
 * go depth 6 moves e2e4
 * go moves e2e4 movetime 300 depth 10
 * */
void parse_go(char *command)
{
    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command,"infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command,"binc")) && side_to_move == black)
        // parse black time increment
        inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command,"winc")) && side_to_move == white)
        // parse white time increment
        inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command,"wtime")) && side_to_move == white)
        // parse white time limit
        time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command,"btime")) && side_to_move == black)
        // parse black time limit
        time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command,"movestogo")))
        // parse number of moves to go
        movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command,"movetime")))
        // parse amount of time allowed to spend to make a move
        movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command,"depth")))
        // parse search depth
        depth = atoi(argument + 6);

    // if move time is not available
    if(movetime != -1)
    {
        // set time equal to move time
        time = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = get_time_ms();

    // init search depth
    depth = depth;

    // if time control is available
    if(time != -1)
    {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        time /= movestogo;

        // "illegal" (empty) move bug fix
        if (time > 1500) time -= 50;

        // init stoptime
        stoptime = starttime + time + inc;
    }

    // if depth is not available
    if(depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
    time, starttime, stoptime, depth, timeset);

    // search position
    search_position(depth);
}


void uci_loop() {
  // clear buffer
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  // define input command (user or gui) length
  char input [2000];

  puts("id name echo");
  puts("id name am-ml");
  puts("uciok");

  while (1) {
    memset(input, 0, sizeof(input)); // clear command input
    fflush(stdout); // ensure output reach

    if (!fgets(input, 2000, stdin)) {
      continue;
    }
    if (input[0] == '\n') continue;

    if (strncmp(input, "isready", 7) == 0) {
      puts("readyok"); continue;
    }

    if (strncmp(input, "position", 8) == 0) {
      parse_position(input); clear_tt();
      continue;
    }

    if (strncmp(input, "makemoves", 9) == 0) {
      parse_uci_makemoves(input);
      continue;
    }

    if (strncmp(input, "ucinewgame", 10) == 0) {
      parse_position("position startpos"); clear_tt();
      continue;
    }

    if (strncmp(input, "go", 2) == 0) {
      parse_go(input); continue;
    }

    if (strncmp(input, "eval", 4) == 0) {
      printf("eval: %d\n", eval()); continue;
    }

    if (strncmp(input, "quit", 4) == 0 ) {
      break; continue;
    }

    if (strncmp(input, "uci", 3) == 0) {
      puts("uciok"); continue;
    }

    if (strncmp(input, "print", 5) == 0) {
      print_board(1); continue;
    }
  }
}


void init_all() {
  init_leaper_attacks();
  init_sliding_pieces(bishop);
  init_sliding_pieces(rook);
  init_default_board_position();
  init_hash_keys();
  init_tt();
  init_pawns_eval_masks();
  // init_magic_numbers();
}


int main(void) {
  init_all();

  parse_fen(start_position);
  uci_loop();

  free(TranspositionTable);
  return 0;
}

