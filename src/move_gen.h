#pragma once

#ifndef _MOVE_GEN_H__
#define _MOVE_GEN_H__

#include "board.h"
#include "tt.h"
#include "masks.h"
#include "sliding_masks.h"

int make_move(int move, int move_flag);
void generate_moves(Moves *moves_list);
void generate_capture_moves(Moves *moves_list);

#endif
