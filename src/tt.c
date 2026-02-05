#include "tt.h"

U64 hash_key;

#pragma omp threadprivate(hash_key)

// Zobrist hashing
U64 piece_keys[12][64];
U64 enpassant_keys[64];
U64 castle_keys[16];
U64 side_to_move_key;


void init_hash_keys () {
  // Constant seed for reproducible keys
  state = 1804289383;

  for(int piece = wP; piece <= bK; piece++) {
    for(int square = 0; square < 64; square++) {
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
  U64 fkey = 0ULL;
  U64 piece_bb;

  // Hash pieces
  for(int piece = wP; piece <= bK; piece++) {
    piece_bb = bitboards[piece];
    while(piece_bb) {
      int piece_square = get_lsb_index(piece_bb);
      fkey ^= piece_keys[piece][piece_square];
      pop_bit(piece_bb, piece_square);
    }
  }

  if (en_passant != no_square) fkey ^= enpassant_keys[en_passant];

  fkey ^= castle_keys[can_castle];
  if (side_to_move == black) fkey ^= side_to_move_key;

  return fkey;
}

// Transposition table
size_t tt_size = TT_SIZE_BYTES / sizeof(TT_Entry);
int16_t age = 0;
TT_Entry* TranspositionTable;

void clear_tt() { memset(TranspositionTable, 0, TT_SIZE_BYTES); }

void init_tt() {
  TranspositionTable = calloc(tt_size, sizeof(TT_Entry));
  if(!TranspositionTable) printf("ERROR! couldn't initialize transposition table.\n");
}

// Probe TT for entry
int probeTT(int alpha, int beta, int depth) {
  size_t index = hash_key & (tt_size - 1);
  TT_Entry* tt_entry = &TranspositionTable[index];

  if (tt_entry->key == hash_key) {
    if (tt_entry->depth >= depth) {
      int score = tt_entry->score;

      // Adjust mate score to be relative to current ply
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

  // Store mate score relative to root
  if (score > MATE_SCORE) score += ply;
  if (score < -MATE_SCORE) score -= ply;

  // Replacement strategy: deeper or new entry
  if (tt_entry->key == 0 || depth >= tt_entry->depth || tt_entry->key != hash_key) {
    tt_entry->key = hash_key;
    tt_entry->depth = (int8_t)depth;
    tt_entry->score = (int16_t)score;
    tt_entry->flag = (int8_t)hashf;

    // Preserve existing move if new move is null
    if (move != 0) tt_entry->move = move;
  }
}

