#include "tt.h"
#include <stdio.h>
#include <string.h>

U64 hash_key;

#pragma omp threadprivate(hash_key)

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

// ----------------------------- //
// ---- TRANSPOSITION TABLE ---- //
// ----------------------------- //

int hash_size_mb = 64; // Default to 64MB
size_t tt_size = 0; // max num entries
int16_t age = 0; // search generation (incremented each search cycle)
TT_Entry* TranspositionTable = NULL;

void clear_tt() {
  if (TranspositionTable) {
    memset(TranspositionTable, 0, tt_size * sizeof(TT_Entry));
  }
}

void resize_tt(int mb) {
  // Calculate maximum entries that fit within the requested MB
  size_t target_size = ((size_t)mb * 1024 * 1024) / sizeof(TT_Entry);

  // Find the highest power of 2 that is <= target_size
  // This is CRITICAL because the engine indexes using & (tt_size - 1)
  tt_size = 1;
  while (tt_size <= target_size) {
      tt_size *= 2;
  }
  tt_size /= 2;

  if (tt_size == 0) tt_size = 1; // Failsafe

  // Free previous table if it exists
  if (TranspositionTable != NULL) {
      free(TranspositionTable);
  }

  // calloc = malloc + memset to 0
  TranspositionTable = calloc(tt_size, sizeof(TT_Entry));
  if(!TranspositionTable) {
      printf("ERROR! couldn't reallocate transposition table to %d MB.\n", mb);
      exit(1);
  }
}

void init_tt() {
  resize_tt(hash_size_mb);
}

// Standardized Mate Score handling
int probeTT(int alpha, int beta, int depth) {
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
