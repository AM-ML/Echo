#pragma once

#ifndef _MAGIC_H__
#define _MAGIC_H__

#include "board.h"
#include "masks.h"

extern U64 rook_magic_numbers[64];
extern U64 bishop_magic_numbers[64];


U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

extern unsigned int state;
extern unsigned int get_random_32();
extern U64 get_random_64();

extern U64 gen_magic_number();
extern U64 find_magic_number(int square, int relevant_bits_count, int flag);

void init_magic_numbers();

#endif
