#pragma once
#ifndef __BOARD_H___
#define __BOARD_H___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>

#include <omp.h>

#ifdef _WIN32
#define FORCE_ASCII 1
#else
#define FORCE_ASCII 0
#endif

#define INFO(output, ...) (printf(#output "\n", __VA_ARGS__))
#define out(output) (printf(#output "\n"))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

extern const int castling_rights[64];

extern char* unicode_pieces[];
extern char ascii_pieces[];
extern int decode_ascii_pieces[];


// define bitboard data type
#define U64 unsigned long long

// rank and file to square
#define RF_2SQ(r, f) ((r) * 8 + (f))

extern U64 bitboards[12];        // pieces bbs
extern U64 sides_occupancies[3]; // sides
extern int piece_on_squares[64]; // mailbox structure: for faster move gen + make, -1 = empty

extern int side_to_move;
extern int can_castle; // WCK WCQ BCQ BCK
extern int en_passant;

#pragma omp threadprivate(bitboards, sides_occupancies, piece_on_squares, side_to_move, can_castle, en_passant)

extern const char *square_to_notation[];
int char_to_square(const char* square);

extern const U64 not_A_file;
extern const U64 not_H_file;
extern const U64 not_HG_file;
extern const U64 not_B_file;
extern const U64 not_AB_file;
extern const U64 not_G_file;
extern const U64 not_rank_1;
extern const U64 not_rank_8;
extern const U64 A_file;
extern const U64 H_file;

extern const U64 rank_1;
extern const U64 rank_2;
extern const U64 rank_7;
extern const U64 rank_8;


// set/get/pop macros
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= 1ULL << (square))
#define pop_bit(bitboard, square)                                              \
  (get_bit((bitboard), (square)) ? ((bitboard) -= 1ULL << (square)) : 0)
#define count_bits(bitboard) (__builtin_popcountll(bitboard))
#define get_lsb(bitboard) ((bitboard) & -(bitboard))
int get_lsb_index(U64 bitboard);

/* ----------------------------------------- */
/* --- rank - file - masks stuff section --- */
/* ----------------------------------------- */

#define get_file(square) ((square) % 8) // s = 8r + f --> 8r % 8 = 0, since file < 8, remainder = file
#define get_rank(square) ((square) / 8) // sqr / 8 = rank.file, remainder of that is cutoff in an integer
#define get_rank_index(square) ((square) >> 3)
#define file_mask(square) (A_file << (get_file(square)))
#define rank_mask(square) (rank_1 >> (get_rank_index(square) * 8))


#define REP_TABLE_SIZE 2048
extern U64 repetition_table[REP_TABLE_SIZE];
extern int repetition_index;

#pragma omp threadprivate(repetition_table, repetition_index)

// position repetition detection
int is_repetition();

void reset_states_and_board();
void set_sides_occupancies();

#define COPY_BOARD()                                                           \
  U64 bitboards_copy[12], sides_occupancies_copy[3], hash_key_copy;            \
  int side_to_move_copy, en_passant_copy, can_castle_copy;                     \
  int piece_on_squares_copy[64];                                               \
  memcpy(bitboards_copy, bitboards, 96);                                       \
  memcpy(sides_occupancies_copy, sides_occupancies, 24);                       \
  memcpy(piece_on_squares_copy, piece_on_squares, 64 * sizeof(int));           \
  side_to_move_copy = side_to_move, en_passant_copy = en_passant,              \
  can_castle_copy = can_castle;                                                \
  hash_key_copy = hash_key;

#define RESTORE_BOARD()                                                        \
  memcpy(bitboards, bitboards_copy, 96);                                       \
  memcpy(sides_occupancies, sides_occupancies_copy, 24);                       \
  memcpy(piece_on_squares, piece_on_squares_copy, 64 * sizeof(int));           \
  side_to_move = side_to_move_copy, en_passant = en_passant_copy,              \
  can_castle = can_castle_copy;                                                \
  hash_key = hash_key_copy

enum { allow_all_moves, allow_only_captures };



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

int is_valid_encoded_move(int move);

typedef struct {
  int moves[256]; // theoretical move limit: 255

  int count;
} Moves;

#define MOVES_CAPACITY 256

int add_move(Moves *move_list, int move);
// --- add move helpers

extern char ascii_promoted_pieces[];

// for UCI purposes
void print_move(int move);
char* get_move_str(int move);

// for debugging purposes
void print_move_list(Moves *move_list);


// order: 8/7/6/5/4/3/2/1 (top to bottom) | 12345678 (left to right) /12345678
void parse_fen(char *fen);

/**********************************\
 ==================================

       Time controls variables

 ==================================
\**********************************/

extern int quit;
extern int movestogo;
extern int movetime;
extern int nodelimit;
extern int time;
extern int inc;
extern int starttime;
extern int stoptime;
extern int timeset;
extern int stopped;


#define MAX_PLY 128
#define MATE_VALUE 49000
#define MATE_SCORE 48000

extern int ply;  // half-move counter

extern int killer_moves[2][MAX_PLY]; // [side][ply]
extern int history_moves[12][64]; // [piece][square]

#pragma omp threadprivate(ply, killer_moves, history_moves)

extern int ponder_move;
extern int pondering;

#endif
