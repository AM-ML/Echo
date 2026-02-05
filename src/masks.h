#pragma once

#ifndef _MASKS_H__
#define _MASKS_H__

#include "board.h"

extern U64 pawn_attacks[2][64];
U64 mask_pawn_attacks(int side, int square);

extern U64 knight_attacks[64];
U64 mask_knight_attacks(int square);

extern U64 king_attacks[64];
U64 mask_king_attacks(int square);

extern U64 bishop_masks[64];
extern U64 bishop_attacks[64][512];
U64 mask_bishop_attacks(int square);
U64 relevant_bishop_attacks(int square, U64 block);

extern U64 rook_masks[64];
extern U64 rook_attacks[64][4096];
U64 mask_rook_attacks(int square);
U64 relevant_rook_attacks(int square, U64 block);

void init_leaper_attacks();

extern const int relevant_knight_count_bits[64];
extern const int relevant_bishop_count_bits[64];
extern const int relevant_rook_count_bits[64];
extern const int relevant_queen_count_bits[64];

extern U64 pawns_file_mask[64];
extern U64 pawns_rank_mask[64];
extern U64 isolated_pawns_mask[64];
extern U64 passed_pawns_mask[2][64];

static inline U64 isolated_pawn_mask(int square);
static inline U64 passed_pawn_mask(int side, int square);

void init_pawns_eval_masks();


extern U64 kingDist2_Mask[64];
U64 mask_king_zone_d2(int square);

#endif
