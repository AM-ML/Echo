#pragma once

#ifndef _EVAL_H__
#define _EVAL_H__

#include "board.h"
#include "masks.h"
#include "sliding_masks.h"

// ---------------------------
// --- PIECE SQUARE TABLES ---
// ---------------------------

// Initialized with White Pieces (indices 0-5).
// Black pieces (indices 6-11) are initialized to 0 and filled by init_black_pst().

// These values include the material score within the PST,
// so we don't need a separate material_score array for the PST calculation logic.

// Middle Game Tables
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
static inline void evaluatePawnStructure(int phase, int *mg_score, int *eg_score);
static inline void evaluateRookActivity(int phase, int *mg_score, int *eg_score);
static inline int manhattan_distance(int sq1, int sq2);
static inline int mopUpEval(int winning_side, int losing_side);
int eval();
U64 get_all_attackers(int square, U64 occupancy);
int see(int move);

#endif
