#include "masks.h"

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
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);
    king_attacks[square] = mask_king_attacks(square);
    kingDist2_Mask[square] = mask_king_zone_d2(square);
  }
}

U64 pawns_file_mask[64];
U64 pawns_rank_mask[64];
U64 isolated_pawns_mask[64];
U64 passed_pawns_mask[2][64];

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
  int WhiteVerticalShifter = get_rank_index(square) * 8;
  int BlackVerticalShifter = (7 - get_rank_index(square)) * 8;
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

// chebyshev distance 2 mask for king safety in the evaluation function
U64 kingDist2_Mask[64];

U64 mask_king_zone_d2(int square) {
    U64 zone = 0ULL;
    int rank = get_rank_index(square);
    int file = get_file(square);

    // Distance-2 Chebyshev includes all squares where max(|dx|, |dy|) <= 2
    for (int dr = -2; dr <= 2; dr++) {
        for (int df = -2; df <= 2; df++) {
            int r = rank + dr;
            int f = file + df;
            // Check bounds and exclude the center square
            if (r >= 0 && r < 8 && f >= 0 && f < 8 && (dr != 0 || df != 0)) {
                set_bit(zone, RF_2SQ(r, f));
            }
        }
    }

    return zone;
}
