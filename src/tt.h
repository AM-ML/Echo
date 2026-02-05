#pragma once

#ifndef _TT_H___
#define _TT_H___

#include "board.h"
#include "magic.h"

extern U64 hash_key;

#pragma omp threadprivate(hash_key)

extern U64 piece_keys[12][64];
extern U64 enpassant_keys[64];
extern U64 castle_keys[16];
extern U64 side_to_move_key;

void init_hash_keys ();
U64 update_hash_key();

#define TT_SIZE_MB 64
#define TT_SIZE_BYTES (TT_SIZE_MB * 1024 * 1024)

#define hashf_EXACT 0
#define hashf_ALPHA 1
#define hashf_BETA  2

#define NO_TT_ENTRY_FOUND 100000

typedef struct {
  U64       key;
  int8_t    depth;
  int8_t    flag;
  int16_t   score;
  int       move;
} TT_Entry;

extern size_t tt_size;
extern int16_t age;
extern TT_Entry* TranspositionTable;

void clear_tt();
void init_tt();

int probeTT(int alpha, int beta, int depth);
int probe_move(void);
void storeTT(int score, int depth, int hashf, int move);

#endif
