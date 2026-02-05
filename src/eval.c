#include "eval.h"

const int DoublePawnPenalty = -15;
const int IsolatedPawnPenalty = -5;
const int ConnectedPassedPawnBonus = 40;

const int RookOpenFileBonus = 15;
const int RookSemiOpenFileBonus = 10;
const int Rook7thRankBonus = 15;
const int Rook7thRankDoubleBonus = 10;

const int UnShieldedKingPenalty = 15;
const int SemiShieldedKingPenalty = 10;

const int KnightOutpostBonus = 25; // Knight defended by pawn
const int BatteryBonus = 10;       // Queen/Rook batteries

const int PawnShieldBonus = 5;
const int ShieldedKingBonus = 5;
const int MissingPawnShieldPenalty = -5;
const int BatteryThreatPenalty = -20;
const int PawnStormPenalty = -15;
const int KingSideShieldedBonus = 15;
const int QueenSideShieldedBonus = 15;

// Passed pawn rank bonuses
const int passed_pawn_bonus[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };

// Piece-Square Tables (including material)
// Initialized for white, mirrored for black.

// Middlegame PSTs
int mg_pst[12][64] = {
    [wP] = {
          0,   0,   0,   0,   0,   0,   0,   0,
         98, 134,  61,  95,  68, 126,  34, -11,
         -6,   7,  26,  31,  65,  56,  25, -20,
        -14,  13,   6,  21,  23,  12,  17, -23,
        -27,  -2,  -5,  12,  17,   6,  10, -25,
        -26,  -4,  -4, -10,   3,   3,  33, -12,
        -35,  -1, -20, -23, -15,  24,  38, -22,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [wN] = {
        -167, -89, -34, -49,  61, -97, -15, -107,
        -73, -41,  72,  36,  23,  62,   7, -17,
        -47,  60,  37,  65,  84, 129,  73,  44,
         -9,  17,  19,  53,  37,  69,  18,  22,
        -13,   4,  16,  13,  28,  19,  21,  -8,
        -23,  -9,  12,  10,  19,  17,  25, -16,
        -29, -53, -12,  -3,  -1,  18, -14, -19,
        -105, -21, -58, -33, -17, -28, -19, -23
    },
    [wB] = {
        -29,   4, -82, -37, -25, -42,   7,  -8,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -16,  37,  43,  40,  35,  50,  37,  -2,
         -4,   5,  19,  50,  37,  37,   7,  -2,
         -6,  13,  13,  26,  34,  12,  10,   4,
          0,  15,  15,  15,  14,  27,  18,  10,
          4,  15,  16,   9,  23,  44,  13, -22,
        -33,  -3, -14, -21, -13, -12, -39, -21
    },
    [wR] = {
         32,  42,  32,  51,  63,   9,  31,  43,
         27,  32,  58,  62,  80,  67,  26,  44,
         -5,  19,  26,  36,  17,  45,  61,  16,
        -24, -11,   7,  26,  24,  35,  -8, -20,
        -36, -26, -12,  -1,   9,  -7,   6, -23,
        -45, -25, -16, -17,   3,   0,  -5, -33,
        -44, -16, -20,  -9,  -1,  11,  -6, -71,
        -19, -13,   1,  17,  16,   7, -37, -26
    },
    [wQ] = {
        -28,   0,  29,  12,  59,  44,  43,  45,
        -24, -39,  -5,   1, -16,  57,  28,  54,
        -13, -17,   7,   8,  29,  56,  47,  57,
        -27, -27, -16, -16,  -1,  17,  -2,   1,
         -9, -26,  -9, -10,  -2,  -4,   3,  -3,
        -14,   2, -11,  -2,  -5,   2,  14,   5,
        -35,  -8,  11,   2,   8,  15,  -3,   1,
         -1, -18,  -9,  10, -15, -25, -31, -50
    },
    [wK] = {
        -65,  23,  16, -15, -56, -34,   2,  13,
         29,  -1, -20,  -7,  -8,  -4, -38, -29,
         -9,  24,   2, -16, -20,   6,  22, -22,
        -17, -20, -12, -27, -30, -25, -14, -36,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -14, -14, -22, -46, -44, -30, -15, -27,
          1,   7,  -8, -64, -43, -16,   9,   8,
        -15,  36,  12, -54,   8, -28,  24,  14
    }
};

// Endgame PSTs
int eg_pst[12][64] = {
    [wP] = {
          0,   0,   0,   0,   0,   0,   0,   0,
        178, 173, 158, 134, 147, 132, 165, 187,
         94, 100,  85, 118, 120,  91,  65,  93,
         32,  24,  13,   5,  -2,   4,  17,  17,
         13,   9,  -3,  -7,  -7,  -8,   3,  -1,
          4,   7,  -6,   1,   0,  -5,  -1,  -8,
         13,   8,   8,  10,  13,   0,   2,  -7,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [wN] = {
        -58, -38, -13, -28, -31, -27, -63, -99,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -29, -51, -23, -15, -22, -18, -50, -64
    },
    [wB] = {
        -14, -21, -11,  -8,  -7,  -9, -17, -24,
         -8,  -4,   7, -12, -3, -13,  -4, -14,
          2,  -8,   0,  -1, -13,   6,   0,   4,
         -3,   9,  12,   4,  14,  12,  -2,   0,
         -6,   3,  13,  19,   7,  10,  -3, -9,
        -12,  -3,   8,  10,  13,   3,  -7, -15,
        -24, -18,  -7,  -2,  -4, -10, -10, -25,
        -23,  -3, -23,  -5, -16, -20, -14, -17
    },
    [wR] = {
         13,  10,  18,  15,  12,  12,   8,   5,
         11,  13,  13,  11,  -3,   3,   8,   3,
          7,   7,   7,   5,   4,  -3,  -5,  -3,
          4,   3,  13,   1,   2,   1,  -1,   2,
          3,   5,   8,   4,  -5,  -6,  -8, -11,
         -4,   0,  -5,  -1,  -7, -12,  -8, -16,
         -6,  -6,   0,   2,  -9,  -9, -11,  -3,
         -9,   2,   3,  -1,  -5, -13,   4, -17
    },
    [wQ] = {
         -9,  22,  22,  27,  27,  19,  10,  20,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -20,   6,   9,  49,  47,  35,  19,   9,
          3,  22,  24,  45,  57,  40,  31,  -7,
        -18,  28,  19,  47,  31,  34,  39,  -6,
        -16, -27,  15,  6,    9,  17,  10,   5,
        -22, -23, -30, -16, -16, -23, -36, -32,
        -33, -28, -22, -43,  -5, -32, -20, -41
    },
    [wK] = {
        -74, -35, -18, -18, -11,  15,   4, -17,
        -12,  17,  14,  17,  17,  38,  23,  11,
         10,  17,  23,  15,  20,  45,  44,  13,
         -8,  22,  24,  27,  26,  33,  26,   3,
        -18,  -4,  21,  24,  27,  23,   9, -11,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -53, -34, -21, -11, -28, -14, -24, -43
    }
};

// Mirror PST for black pieces
void init_black_pst() {
  for (int piece = wP; piece <= wK; piece++) {
    int black_piece = piece + 6; // wP=0 -> bP=6

    for (int sq = 0; sq < 64; sq++) {
      // Mirror square vertically: 0 (A8) <-> 56 (A1)
      int mirror_sq = sq ^ 56;

      // Negate the value
      mg_pst[black_piece][mirror_sq] = -mg_pst[piece][sq];
      eg_pst[black_piece][mirror_sq] = -eg_pst[piece][sq];
    }
  }
}

// PeSTO material values (middlegame)
int material_score_mg[12] = {
       90, // wP
      547, // wN
      578, // wB
      893, // wR
     1777, // wQ
    16000, // wK
      -90, // bP
     -547, // bN
     -578, // bB
     -893, // bR
    -1777, // bQ
   -16000  // bK
};

// PeSTO material values (endgame)
int material_score_eg[12] = {
      149, // wP
      598, // wN
      641, // wB
      966, // wR
     1878, // wQ
    16000, // wK
     -149, // bP
     -598, // bN
     -641, // bB
     -966, // bR
    -1878, // bQ
   -16000  // bK
};

// Embed material scores into PST
void init_evaluation() {
  for (int piece = wP; piece <= bK; piece++) {
    for (int sq = 0; sq < 64; sq++) {
      mg_pst[piece][sq] += material_score_mg[piece];
      eg_pst[piece][sq] += material_score_eg[piece];
    }
  }
}


// MVV/LVA table
int mvv_lva[12][12] = {
  {105, 104, 103, 102, 101, 100, 105, 104, 103, 102, 101, 100},
  {205, 204, 203, 202, 201, 200, 205, 204, 203, 202, 201, 200},
  {305, 304, 303, 302, 301, 300, 305, 304, 303, 302, 301, 300},
  {405, 404, 403, 402, 401, 400, 405, 404, 403, 402, 401, 400},
  {505, 504, 503, 502, 501, 500, 505, 504, 503, 502, 501, 500},
  {605, 604, 603, 602, 601, 600, 605, 604, 603, 602, 601, 600},

  // Mirror for black... (indices 6-11) - mapping [victim][attacker]
  {105, 104, 103, 102, 101, 100, 105, 104, 103, 102, 101, 100},
  {205, 204, 203, 202, 201, 200, 205, 204, 203, 202, 201, 200},
  {305, 304, 303, 302, 301, 300, 305, 304, 303, 302, 301, 300},
  {405, 404, 403, 402, 401, 400, 405, 404, 403, 402, 401, 400},
  {505, 504, 503, 502, 501, 500, 505, 504, 503, 502, 501, 500},
  {605, 604, 603, 602, 601, 600, 605, 604, 603, 602, 601, 600}
};

// Center Manhattan Distance for mop-up
const int cmd_score[64] = {
    200, 150, 100,  50,  50, 100, 150, 200,
    150, 100,  50,  20,  20,  50, 100, 150,
    100,  50,  20,  10,  10,  20,  50, 100,
     50,  20,  10,   0,   0,  10,  20,  50,
     50,  20,  10,   0,   0,  10,  20,  50,
    100,  50,  20,  10,  10,  20,  50, 100,
    150, 100,  50,  20,  20,  50, 100, 150,
    200, 150, 100,  50,  50, 100, 150, 200
};


int piecePhaseWeights[12] = {
  [wP] = 0, [wN] = 10, [wB] = 10, [wR] = 20, [wQ] = 40, [wK] = 0,
  [bP] = 0, [bN] = 10, [bB] = 10, [bR] = 20, [bQ] = 40, [bK] = 0,
};

const int opening_phase = 256;
const int endgame_phase = 0;


static inline int calculatePhaseFactor() {
  int phase = 0;
  phase += count_bits(bitboards[wN]) * piecePhaseWeights[wN];
  phase += count_bits(bitboards[bN]) * piecePhaseWeights[bN];

  phase += count_bits(bitboards[wB]) * piecePhaseWeights[wB];
  phase += count_bits(bitboards[bB]) * piecePhaseWeights[bB];

  phase += count_bits(bitboards[wR]) * piecePhaseWeights[wR];
  phase += count_bits(bitboards[bR]) * piecePhaseWeights[bR];

  phase += count_bits(bitboards[wQ]) * piecePhaseWeights[wQ];
  phase += count_bits(bitboards[bQ]) * piecePhaseWeights[bQ];

  return (phase > 256)? 256 : phase; // 0 = endgame -> 1 = middlegame
}

// King safety logic

// Attacker weights by piece type
const int attackerWeight[] = { 0, 20, 20, 40, 80, 0, 0, 20, 20, 40, 80, 0 };

// Attacker count penalty scale
const int attackersPenalty[16] = {
  0, 0, 10, 30, 60, 90, 130, 170, 230, 300, 380, 470, 570, 680, 800, 930
};

// King safety zone (square + adjacent)
static inline U64 getKingZone(int king_sq) {
  return king_attacks[king_sq] | (1ULL << king_sq);
}

static inline int isCastledKingside(int king_square, int side) {
  if (side == white) return (king_square == g1);
  return king_square == g8;
}

static inline int isCastledQueenside(int king_square, int side) {
  if (side == white) return (king_square == c1);
  return king_square == c8;
}

// to add later with the mg_score in the evaluation function
static inline int evaluateKingSafety(int phase) {
  int safety_score = 0;

  // Quick phase check - skip if too close to endgame
  if (phase < 100) return 0; // Even more aggressive gating

  int w_king_sq = get_lsb_index(bitboards[wK]);
  int b_king_sq = get_lsb_index(bitboards[bK]);

  U64 w_king_zone = king_attacks[w_king_sq] | bitboards[wK];
  U64 b_king_zone = king_attacks[b_king_sq] | bitboards[bK];

  // Pre-fetch all piece bitboards (cache-friendly)
  U64 w_pawns = bitboards[wP];
  U64 b_pawns = bitboards[bP];
  U64 occ = sides_occupancies[both];

  // White king safety
  int w_attackers = 0, w_weight = 0;

  if (bitboards[bQ] | bitboards[bR]) {
    // Knights
    U64 bb = bitboards[bN];
    while (bb) {
      if (knight_attacks[get_lsb_index(bb)] & w_king_zone) {
        w_attackers++;
        w_weight += 20;
      }
      pop_bit(bb, get_lsb_index(bb));
    }

    // Bishops (only if they exist)
    bb = bitboards[bB];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_bishop_attacks(sq, occ) & w_king_zone) {
          w_attackers++;
          w_weight += 20;
        }
        pop_bit(bb, sq);
      }
    }

    // Rooks (only if they exist)
    bb = bitboards[bR];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_rook_attacks(sq, occ) & w_king_zone) {
          w_attackers++;
          w_weight += 40;
        }
        pop_bit(bb, sq);
      }
    }

    // Queens (only if they exist)
    bb = bitboards[bQ];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_queen_attacks(sq, occ) & w_king_zone) {
          w_attackers++;
          w_weight += 80;
        }
        pop_bit(bb, sq);
      }
    }
  }

  if (w_attackers > 1) {
    int idx = (w_attackers > 15) ? 15 : w_attackers;
    safety_score -= (w_weight + attackersPenalty[idx]);
  }

  // Black king safety
  int b_attackers = 0, b_weight = 0;

  if (bitboards[wQ] | bitboards[wR]) {
    U64 bb = bitboards[wN];
    while (bb) {
      if (knight_attacks[get_lsb_index(bb)] & b_king_zone) {
        b_attackers++;
        b_weight += 20;
      }
      pop_bit(bb, get_lsb_index(bb));
    }

    bb = bitboards[wB];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_bishop_attacks(sq, occ) & b_king_zone) {
          b_attackers++;
          b_weight += 20;
        }
        pop_bit(bb, sq);
      }
    }

    bb = bitboards[wR];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_rook_attacks(sq, occ) & b_king_zone) {
          b_attackers++;
          b_weight += 40;
        }
        pop_bit(bb, sq);
      }
    }

    bb = bitboards[wQ];
    if (bb) {
      while (bb) {
        int sq = get_lsb_index(bb);
        if (get_queen_attacks(sq, occ) & b_king_zone) {
          b_attackers++;
          b_weight += 80;
        }
        pop_bit(bb, sq);
      }
    }
  }

  if (b_attackers > 1) {
    int idx = (b_attackers > 15) ? 15 : b_attackers;
    safety_score += (b_weight + attackersPenalty[idx]);
  }

  // Pawn shield evaluation
  int w_is_kingside = (w_king_sq == g1);
  int w_is_queenside = (w_king_sq == c1);

  if (w_is_kingside) {
    // Check f2, g2, h2 (rank 6)
    U64 shield = (1ULL << RF_2SQ(6, 5)) | (1ULL << RF_2SQ(6, 6)) | (1ULL << RF_2SQ(6, 7));
    int cnt = count_bits(w_pawns & shield);
    if (cnt == 3) safety_score += KingSideShieldedBonus;
    safety_score += cnt * PawnShieldBonus + (3 - cnt) * MissingPawnShieldPenalty;

    // Pawn storm check
    U64 storm = (1ULL << RF_2SQ(1, 5)) | (1ULL << RF_2SQ(1, 6)) | (1ULL << RF_2SQ(1, 7));
    safety_score -= (3 - count_bits(b_pawns & storm)) * PawnStormPenalty;

  } else if (w_is_queenside) {
    U64 shield = (1ULL << RF_2SQ(6, 0)) | (1ULL << RF_2SQ(6, 1)) | (1ULL << RF_2SQ(6, 2));
    int cnt = count_bits(w_pawns & shield);
    if (cnt == 3) safety_score += QueenSideShieldedBonus;
    safety_score += cnt * PawnShieldBonus + (3 - cnt) * MissingPawnShieldPenalty;

    U64 storm = (1ULL << RF_2SQ(1, 0)) | (1ULL << RF_2SQ(1, 1)) | (1ULL << RF_2SQ(1, 2));
    safety_score += (3 - count_bits(b_pawns & storm)) * PawnStormPenalty;
  }

  // BLACK
  int b_is_kingside = (b_king_sq == g8);
  int b_is_queenside = (b_king_sq == c8);

  if (b_is_kingside) {
    U64 shield = (1ULL << RF_2SQ(1, 5)) | (1ULL << RF_2SQ(1, 6)) | (1ULL << RF_2SQ(1, 7));
    int cnt = count_bits(b_pawns & shield);
    if (cnt == 3) safety_score -= KingSideShieldedBonus;
    safety_score -= cnt * PawnShieldBonus + (3 - cnt) * MissingPawnShieldPenalty;

    U64 storm = (1ULL << RF_2SQ(6, 5)) | (1ULL << RF_2SQ(6, 6)) | (1ULL << RF_2SQ(6, 7));
    safety_score -= (3 - count_bits(w_pawns & storm)) * PawnStormPenalty;

  } else if (b_is_queenside) {
    U64 shield = (1ULL << RF_2SQ(1, 0)) | (1ULL << RF_2SQ(1, 1)) | (1ULL << RF_2SQ(1, 2));
    int cnt = count_bits(b_pawns & shield);
    if (cnt == 3) safety_score -= QueenSideShieldedBonus;
    safety_score -= cnt * PawnShieldBonus + (3 - cnt) * MissingPawnShieldPenalty;

    U64 storm = (1ULL << RF_2SQ(6, 0)) | (1ULL << RF_2SQ(6, 1)) | (1ULL << RF_2SQ(6, 2));
    safety_score -= (3 - count_bits(w_pawns & storm)) * PawnStormPenalty;
  }

  // Battery detection (Queen/Rook, Queen/Bishop)
  if (phase > 100) {
    // White threats
    U64 w_queens = bitboards[wQ];
    if (w_queens && (bitboards[wR] | bitboards[wB])) {
      int q_sq = get_lsb_index(w_queens);
      if (get_queen_attacks(q_sq, occ) & b_king_zone) {
        // Check for rook battery (same rank/file)
        U64 rooks = bitboards[wR];
        int q_rank = get_rank_index(q_sq);
        int q_file = get_file(q_sq);

        while (rooks) {
          int r_sq = get_lsb_index(rooks);
          if ((get_rank_index(r_sq) == q_rank || get_file(r_sq) == q_file) &&
            (get_rook_attacks(r_sq, occ) & (1ULL << q_sq))) {
            safety_score -= BatteryThreatPenalty;
            break; // Only penalize once
          }
          pop_bit(rooks, r_sq);
        }

        // Check for bishop battery (same diagonal)
        U64 bishops = bitboards[wB];
        while (bishops) {
          int b_sq = get_lsb_index(bishops);
          int rank_diff = abs(get_rank_index(b_sq) - q_rank);
          int file_diff = abs(get_file(b_sq) - q_file);
          if (rank_diff == file_diff && rank_diff > 0 &&
            (get_bishop_attacks(b_sq, occ) & (1ULL << q_sq))) {
            safety_score -= BatteryThreatPenalty;
            break;
          }
          pop_bit(bishops, b_sq);
        }
      }
    }

    // Black threats to white king (symmetric)
    U64 b_queens = bitboards[bQ];
    if (b_queens && (bitboards[bR] | bitboards[bB])) {
      int q_sq = get_lsb_index(b_queens);
      if (get_queen_attacks(q_sq, occ) & w_king_zone) {
        U64 rooks = bitboards[bR];
        int q_rank = get_rank_index(q_sq);
        int q_file = get_file(q_sq);

        while (rooks) {
          int r_sq = get_lsb_index(rooks);
          if ((get_rank_index(r_sq) == q_rank || get_file(r_sq) == q_file) &&
            (get_rook_attacks(r_sq, occ) & (1ULL << q_sq))) {
            safety_score += BatteryThreatPenalty;
            break;
          }
          pop_bit(rooks, r_sq);
        }

        U64 bishops = bitboards[bB];
        while (bishops) {
          int b_sq = get_lsb_index(bishops);
          int rank_diff = abs(get_rank_index(b_sq) - q_rank);
          int file_diff = abs(get_file(b_sq) - q_file);
          if (rank_diff == file_diff && rank_diff > 0 &&
            (get_bishop_attacks(b_sq, occ) & (1ULL << q_sq))) {
            safety_score += BatteryThreatPenalty;
            break;
          }
          pop_bit(bishops, b_sq);
        }
      }
    }
  }

  return safety_score;
}

// Bonus for having the right to move (Tempo)
const int tempo_bonus = 20;

// Weighted Mobility: (Count - Offset) * Weight
// If result is negative, it becomes a penalty.
// Rooks need ~4 squares to be happy    N    B    R    Q
const int mobility_bonus_offset[4] = {  0,   0,   4,   2 };

// Bishops benefit most from open diagonals
const int mobility_bonus_weight[4] = {  0,   5,   2,   1 };

static inline int isOpenFile(int file_index) {
  U64 mask = A_file << file_index;
  return !((mask & bitboards[wP]) || (mask & bitboards[bP]));
}

static inline int isPassedPawn(int side, int square) {
  U64 enemy_pawns = (side == white) ? bitboards[bP] : bitboards[wP];
  return !(passed_pawns_mask[side][square] & enemy_pawns);
}

static inline int evaluatePawnStructure(int phase) {
  int score = 0;
  U64 bitboard;
  int square;

  // --- WHITE PAWNS ---
  bitboard = bitboards[wP];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    // Passed Pawn Check
    // If no black pawns are in front or on adjacent files
    if (!(passed_pawns_mask[white][square] & bitboards[bP])) {
      score += passed_pawn_bonus[7 - get_rank_index(square)];

      // Connected Passed Pawn Bonus
      // Condition: 1. Defended by a friendly pawn
      //            2. That defender is ALSO a passed pawn

      U64 defenders = pawn_attacks[black][square] & bitboards[wP];

      if (defenders) {
        while (defenders) {
          int defender_sq = get_lsb_index(defenders);
          // Check if defender is passed
          if (!(passed_pawns_mask[white][defender_sq] & bitboards[bP])) {
            score += ConnectedPassedPawnBonus;
            break; // Bonus added once per connection structure
          }
          pop_bit(defenders, defender_sq);
        }
      }
    }

    // 2. Isolated Pawn Check
    if (!(isolated_pawns_mask[square] & bitboards[wP])) {
      score += IsolatedPawnPenalty;
    }

    pop_bit(bitboard, square);
  }

  // --- BLACK PAWNS ---
  bitboard = bitboards[bP];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    // Passed Pawn Check
    if (!(passed_pawns_mask[black][square] & bitboards[wP])) {
      score -= passed_pawn_bonus[get_rank_index(square)];

      // Connected Passed Pawn Bonus
      U64 defenders = pawn_attacks[white][square] & bitboards[bP];

      if (defenders) {
        while (defenders) {
          int defender_sq = get_lsb_index(defenders);
          if (!(passed_pawns_mask[black][defender_sq] & bitboards[wP])) {
            score -= ConnectedPassedPawnBonus;
            break;
          }
          pop_bit(defenders, defender_sq);
        }
      }
    }

    // Isolated Pawn Penatly
    if (!(isolated_pawns_mask[square] & bitboards[bP])) {
      score -= IsolatedPawnPenalty;
    }

    pop_bit(bitboard, square);
  }

  return score;
}

static inline int evaluateRookActivity(int phase) {
  int score = 0;
  U64 bitboard;
  int square;

  // --- WHITE ROOKS ---
  bitboard = bitboards[wR];
  int w_infiltration_count = 0;

  // Infiltration Condition: Enemy King on Back Rank (Rank 8 / Index 0-7)
  U64 b_king_on_backrank = (bitboards[bK] & rank_8);

  while (bitboard) {
    square = get_lsb_index(bitboard);

    // Mobility/Open Files
    if (!(pawns_file_mask[square] & bitboards[wP])) {
      score += RookSemiOpenFileBonus;
      if (!(pawns_file_mask[square] & bitboards[bP])) {
        score += RookOpenFileBonus;
      }
    }

    // Infiltration Logic (Rank 7 is indices 8-15)
    if (get_rank_index(square) == 1) { // Rank 7 (rows 0-7, 8-15...) -> Index 1 is Rank 7
      // Condition: Pawns on Rank 7 OR King on Rank 8
      if ((bitboards[bP] & rank_7) || b_king_on_backrank) {
        score += Rook7thRankBonus;
        w_infiltration_count++;
      }
    }

    pop_bit(bitboard, square);
  }
  // Double Infiltration Bonus
  if (w_infiltration_count > 1) score += Rook7thRankDoubleBonus;


  // --- BLACK ROOKS ---
  bitboard = bitboards[bR];
  int b_infiltration_count = 0;

  // Infiltration Condition: Enemy King on Back Rank (Rank 1 / Index 56-63)
  U64 w_king_on_backrank = (bitboards[wK] & rank_1);

  while (bitboard) {
    square = get_lsb_index(bitboard);

    // Mobility/Open Files
    if (!(pawns_file_mask[square] & bitboards[bP])) {
      score -= RookSemiOpenFileBonus;
      if (!(pawns_file_mask[square] & bitboards[wP])) {
        score -= RookOpenFileBonus;
      }
    }

    // Infiltration Logic (Rank 2 is indices 48-55)
    if (get_rank_index(square) == 6) { // Rank 2
      // Condition: Pawns on Rank 2 OR King on Rank 1
      if ((bitboards[wP] & rank_2) || w_king_on_backrank) {
        score -= Rook7thRankBonus;
        b_infiltration_count++;
      }
    }

    pop_bit(bitboard, square);
  }
  // Double Infiltration Bonus
  if (b_infiltration_count >= 2) score -= Rook7thRankDoubleBonus;


  // --- BATTERY BONUS ---
  // Scan files 0-7 once. This is faster than piece-centric loops for column checks.
  // White Battery: Needs Black King on Rank 8
  if (b_king_on_backrank) {
    for (int file = 0; file < 8; file++) {
      // Condition: Open File
      if (isOpenFile(file)) {
        U64 file_m = A_file << file;
        int rooks = count_bits(bitboards[wR] & file_m);
        int queens = count_bits(bitboards[wQ] & file_m);

        // Condition: Q+R or R+R (Total >= 2, with at least 1 Rook)
        if (rooks >= 2 || (rooks >= 1 && queens >= 1)) {
          score += BatteryBonus;
        }
      }
    }
  }

  // Black Battery: Needs White King on Rank 1
  if (w_king_on_backrank) {
    for (int file = 0; file < 8; file++) {
      // Condition: Open File
      if (isOpenFile(file)) {
        U64 file_m = A_file << file;
        int rooks = count_bits(bitboards[bR] & file_m);
        int queens = count_bits(bitboards[bQ] & file_m);

        if (rooks >= 2 || (rooks >= 1 && queens >= 1)) {
          score -= BatteryBonus;
        }
      }
    }
  }

  return score;
}

// Manhattan distance between two squares
static inline int manhattan_distance(int sq1, int sq2) {
    int file1 = get_file(sq1);
    int rank1 = get_rank_index(sq1);
    int file2 = get_file(sq2);
    int rank2 = get_rank_index(sq2);
    return abs(rank2 - rank1) + abs(file2 - file1);
}

// Mop-up evaluation for endgames
static inline int mopUpEval(int winning_side, int losing_side) {
    int winning_king_sq = get_lsb_index(bitboards[winning_side == white ? wK : bK]);
    int losing_king_sq = get_lsb_index(bitboards[losing_side == white ? wK : bK]);

    int score = 0;

    // Center Manhattan Distance (CMD) of losing King
    // cmd_score[] gives ~200 bonus for edges and 0 for center.
    // This perfectly matches the goal of pushing King to the edge.
    score += cmd_score[losing_king_sq];

    // Manhattan Distance (MD) between Kings
    // We want to minimize distance. Max MD is ~14.
    // Formula: (14 - dist) * Weight.
    int md = manhattan_distance(winning_king_sq, losing_king_sq);
    score += (14 - md) * 10; // max score = 130

    return score;
}

// Static evaluation function
int eval() {
  int mg_score = 0;
  int eg_score = 0;
  int score = 0;

  // Material and PST evaluation
  int piece;
  U64 bitboard;
  int square;

  for (piece = wP; piece <= bK; piece++) {
    bitboard = bitboards[piece];
    while (bitboard) {
      square = get_lsb_index(bitboard);
      mg_score += mg_pst[piece][square];
      eg_score += eg_pst[piece][square];
      pop_bit(bitboard, square);
    }
  }

  int phase = calculatePhaseFactor();

  // Pawn Structure (Passed, Isolated, Connected, Doubled)
  score += evaluatePawnStructure(phase);

  // King Safety & Rook Activation (Only in Middlegame)
  // phase > 50 is a clever cut-off to save speed
  // but yields some risk.
  if (phase > 50) {
    mg_score += evaluateKingSafety(phase);
    mg_score += evaluateRookActivity(phase);
  }

  // Mobility & Piece Specifics

  U64 occ = sides_occupancies[both];

  // Bishops Mobility

  // White
  bitboard = bitboards[wB];
  while(bitboard) {
    square = get_lsb_index(bitboard);

    // exponential mobility bonus
    score +=( count_bits(get_bishop_attacks(square, occ)) - mobility_bonus_offset[1] )
            * mobility_bonus_weight[1];

    pop_bit(bitboard, square);
  }

  // Black
  bitboard = bitboards[bB];
  while(bitboard) {
    square = get_lsb_index(bitboard);

    score -=( count_bits(get_bishop_attacks(square, occ)) - mobility_bonus_offset[1] )
            * mobility_bonus_weight[1];

    pop_bit(bitboard, square);
  }

  // Queens Mobility

  // White
  bitboard = bitboards[wQ];
  while(bitboard) {
    square = get_lsb_index(bitboard);

    score +=( count_bits(get_queen_attacks(square, occ)) - mobility_bonus_offset[3])
            * mobility_bonus_weight[3];

    pop_bit(bitboard, square);
  }

  // Black
  bitboard = bitboards[bQ];
  while(bitboard) {
    square = get_lsb_index(bitboard);

    score -=( count_bits(get_queen_attacks(square, occ)) - mobility_bonus_offset[3])
            * mobility_bonus_weight[3];

    pop_bit(bitboard, square);
  }

  // Knight Outposts

  // White
  U64 w_knights = bitboards[wN];
  while (w_knights) {
    int sq = get_lsb_index(w_knights);
    int r = get_rank_index(sq);
    // effective on 3rd, 4th, 5th, 6th ranks for white if not attackable & defended
    if (r >= 2 && r <= 5 && (pawn_attacks[black][sq] & bitboards[wP])) {
      if (!(passed_pawns_mask[white][sq] & bitboards[bP])) mg_score += KnightOutpostBonus;
    }
    pop_bit(w_knights, sq);
  }

  // Black
  U64 b_knights = bitboards[bN];
  while (b_knights) {
    int sq = get_lsb_index(b_knights);
    int r = get_rank_index(sq);
    if (r >= 2 && r <= 5 && (pawn_attacks[white][sq] & bitboards[bP])) {
      if (!(passed_pawns_mask[black][sq] & bitboards[wP])) mg_score -= KnightOutpostBonus;
    }
    pop_bit(b_knights, sq);
  }

  // Interpolation between middlegame and endgame
  int final_score = ((mg_score * phase) + (eg_score * (256 - phase))) / 256;
  final_score += score;

  // Mop Up Evaluation (for K+Q / K+R vs. K checkmates)
  if (phase < 50) { // deep into the endgame
    if (final_score > 200) {
      final_score += mopUpEval(white, black);
    } else if (final_score < -200) {
      final_score -= mopUpEval(black, white);
    }
  }


  // Tempo bonus
  final_score += (side_to_move == white) ? tempo_bonus : -tempo_bonus;
  return (side_to_move == white) ? final_score : -final_score;
}
