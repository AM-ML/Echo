#pragma once

#ifndef _SEARCH_H__
#define _SEARCH_H__

#include "tt.h"
#include "uci.h"
#include "eval.h"
#include "board.h"
#include "move_gen.h"

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define INF 1000000
#define NEG_INF -1000000

int get_time_ms();
extern U64 nodes;

static inline void perft_driver(int depth);
void perft_test(int depth);

extern int pv_length[MAX_PLY];
extern int pv_table[MAX_PLY][MAX_PLY];

extern int apply_pv, pv_score;


static inline void enable_pv_scoring(Moves* ml);
static inline int score_move(int move, int tt_move);
static inline void sort_moves(Moves *ml, int tt_move);
static inline int quiescence_search(int alpha, int beta, int qs_depth);
static inline int negamax(int alpha, int beta, int depth);
void search_position(int depth);

#endif
