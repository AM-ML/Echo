#pragma once

#ifndef _TT_H___
#define _TT_H___

#include "board.h"
#include "magic.h"

extern U64 hash_key;

#pragma omp threadprivate(hash_key)

// ---------------------------
// ----- ZOBRIST HASHING -----
// ---------------------------


extern U64 piece_keys[12][64];
extern U64 enpassant_keys[64]; // [square]
extern U64 castle_keys[16]; // 1111 KQkq = 16
extern U64 side_to_move_key; // white : black (0, 1)

void init_hash_keys ();
U64 update_hash_key();

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

extern size_t tt_size; // max num entries
extern int16_t age; // search generation (incremented each search cycle)
extern TT_Entry* TranspositionTable;

void clear_tt();
void init_tt();

int probeTT(int alpha, int beta, int depth);
int probe_move(void);
void storeTT(int score, int depth, int hashf, int move);

#endif
