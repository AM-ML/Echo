#pragma once

#ifndef _EVAL_H__
#define _EVAL_H__

#include "board.h"
#include "masks.h"
#include "sliding_masks.h"

extern int mg_pst[12][64];
extern int eg_pst[12][64];

extern int material_score_mg[12];
extern int material_score_eg[12];
extern int mvv_lva[12][12];

extern int piecePhaseWeights[12];

extern const int cmd_score[64];

#define ABS(n) ((n) < 0? -(n) : (n))

void init_evaluation();
void init_black_pst();

static inline int calculatePhaseFactor();
static inline U64 getKingZone(int king_sq);
static inline int isCastledKingside(int king_square, int side);
static inline int isCastledQueenside(int king_square, int side);
static inline int evaluateKingSafety(int phase);
static inline int isOpenFile(int file_index);
static inline int isPassedPawn(int side, int square);
static inline int evaluatePawnStructure(int phase);
static inline int evaluateRookActivity(int phase);
static inline int manhattan_distance(int sq1, int sq2);
static inline int mopUpEval(int winning_side, int losing_side);
int eval();

#endif
