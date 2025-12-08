#include "sliding_masks.h"

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

U64 get_bishop_attacks(int square, U64 blockers) {
  blockers &= bishop_masks[square];
  blockers *= bishop_magic_numbers[square];
  blockers >>= 64 - relevant_bishop_count_bits[square];

  return bishop_attacks[square][blockers];
}

U64 get_rook_attacks(int square, U64 blockers) {
  blockers &= rook_masks[square];
  blockers *= rook_magic_numbers[square];
  blockers >>= 64 - relevant_rook_count_bits[square];

  return rook_attacks[square][blockers];
}

U64 get_queen_attacks(int square, U64 blockers) {
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

int is_square_attacked_by(int square, int side) {
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

U64 get_attacked_squares_by(int side) {
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

