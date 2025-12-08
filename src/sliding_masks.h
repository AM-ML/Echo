#pragma once
#ifndef _SLIDING_MASKS_H___
#define _SLIDING_MASKS_H___

#include "board.h"
#include "magic.h"

void init_sliding_pieces(int flag);

// get attacks from magic index
// #define get_bishop_attacks(square, blockers) (blockers = (((blockers &
// bishop_masks[square]) * bishop_magic_numbers[square])) >> (64 -
// relevant_bishop_count_bits[square]))

// #define get_rook_attacks(square, blockers) (blockers = (((blockers &
// rook_masks[square]) * rook_magic_numbers[square])) >> (64 -
// relevant_rook_count_bits[square]))
U64 get_bishop_attacks(int square, U64 blockers);
U64 get_rook_attacks(int square, U64 blockers);
U64 get_queen_attacks(int square, U64 blockers);

int is_square_attacked_by(int square, int side);
U64 get_attacked_squares_by(int side);

#endif
