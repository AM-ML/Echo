#include "search.h"

int get_time_ms() {
#if defined(_WIN64) || defined(_WIN32)
  return GetTickCount();
#else
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return (int)time_value.tv_sec * 1000 + (int)time_value.tv_usec / 1000;
#endif
}

U64 nodes;

static inline void perft_driver(int depth) {
  if (depth == 0) {
    nodes++;
    return;
  }

  Moves ml;
  ml.count = 0;

  generate_moves(&ml);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    COPY_BOARD();

    if (!make_move(move, allow_all_moves)) {
      continue;
    }
    perft_driver(depth - 1); // call perft recursively

    RESTORE_BOARD();


    // U64 whole_hash_key = update_hash_key();
    // if (hash_key != whole_hash_key){
    //   printf("\033[1;93mPERFT DRIVER\033[0;0m move: %s\n", get_move_str(move));
    //   printf("\033[1;36m%llx\033[0;0m should be %llx\n\n", hash_key, whole_hash_key);
    //
    // }
  }
}

void perft_test(int depth) {

  printf("\n  \033[1;92mPerformance Test\033[0;0m\n\n");

  Moves ml;
  ml.count = 0;

  generate_moves(&ml);

  for (int i = 0; i < ml.count; i++) {
    COPY_BOARD();

    if (!make_move(ml.moves[i], allow_all_moves)) {
      continue;
    }

    U64 prev_nodes = nodes;


    perft_driver(depth - 1);

    U64 new_nodes = nodes - prev_nodes;

    RESTORE_BOARD();

    printf("    \033[1;93mmove: \033[0;0m");
    printf("%s-%s%c \033[1;93mnodes: \033[0;0m%llu\n",
           square_to_notation[get_move_source(ml.moves[i])],
           square_to_notation[get_move_target(ml.moves[i])],
           ascii_promoted_pieces[get_move_promoted_piece(ml.moves[i])],
           new_nodes);
  }
}

int pv_length[MAX_PLY];
int pv_table[MAX_PLY][MAX_PLY];

int apply_pv, pv_score;

static inline void enable_pv_scoring(Moves* ml) {
  apply_pv = 0; // reset pv detection flag

  for(int i = 0; i < ml -> count; i++) {
    if(pv_table[0][ply] == ml -> moves[i]) {
      pv_score = 1;

      apply_pv = 1;
    }
  }
}

static inline int score_move(int move, int tt_move) {
  // 1. PV/TT Move has highest priority
  if (move == tt_move) return 30000;

  // 2. Captures (MVV/LVA)
  if (get_move_capture_flag(move)) {
    int target_piece = wP;
    int start_piece = get_move_piece(move);

    if (get_move_en_passant_flag(move)) {
      target_piece = (side_to_move == white) ? bP : wP;
    } else {
      // Find the victim piece
      int victim_start = (side_to_move == white) ? bP : wP;
      int victim_end = (side_to_move == white) ? bK : wK;
      for (int bb_piece = victim_start; bb_piece <= victim_end; bb_piece++) {
        if (get_bit(bitboards[bb_piece], get_move_target(move))) {
          target_piece = bb_piece;
          break;
        }
      }
    }
    // Score: 10000 + MVV[victim][attacker]
    return 10000 + mvv_lva[target_piece][start_piece];
  }
  // 3. Quiet Moves
  else {
    if (killer_moves[0][ply] == move) return 9000;
    else if (killer_moves[1][ply] == move) return 8000;
    else return history_moves[get_move_piece(move)][get_move_target(move)];
  }
}


static inline void sort_moves(Moves *ml, int tt_move) {
  int scores[MOVES_CAPACITY];
  for (int i = 0; i < ml->count; i++)
    scores[i] = score_move(ml->moves[i], tt_move);

  for (int i = 1; i < ml->count; i++) {
    int key_s = scores[i];
    int key_m = ml->moves[i];
    int j = i - 1;
    while (j >= 0 && scores[j] < key_s) {
      scores[j+1] = scores[j];
      ml->moves[j+1] = ml->moves[j];
      j--;
    }
    scores[j+1] = key_s;
    ml->moves[j+1] = key_m;
  }
}

static inline int quiescence_search(int alpha, int beta, int qs_depth) {
  if ((nodes & 2047) == 0) communicate();
  nodes++;

  int eval_score = eval();

  // Hard Depth Limit
  if (ply >= MAX_PLY - 1 || qs_depth <= -15) return eval_score;

  if (eval_score >= beta) return beta;
  if (eval_score > alpha) alpha = eval_score;

  Moves ml;
  generate_capture_moves(&ml);
  sort_moves(&ml, 0);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    // --- Delta Pruning ---
    if (!get_move_promoted_piece(move)) {
      int target_piece = wP;

      if (get_move_en_passant_flag(move)) {
        target_piece = wP;
      } else {
        int start = (side_to_move == white) ? bP : wP;
        int end = (side_to_move == white) ? bK : wK;
        for (int bb = start; bb <= end; bb++) {
          if (get_bit(bitboards[bb], get_move_target(move))) {
            target_piece = bb; break;
          }
        }
      }

      if (eval_score + ABS(material_score[target_piece]) + 200 < alpha) continue;
    }

    COPY_BOARD();
    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (make_move(move, allow_only_captures) == 0) {
      ply--; repetition_index--;
      continue;
    }

    int score = -quiescence_search(-beta, -alpha, qs_depth - 1);

    ply--; repetition_index--;
    RESTORE_BOARD();

    if (stopped) return 0;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

// Enhanced Negamax with improved LMR and extensions
static inline int negamax(int alpha, int beta, int depth) {
  // 1. PV Node Initialization
  pv_length[ply] = ply;
  int pv_node = (beta - alpha) > 1;

  // 2. Base Cases
  if (ply && is_repetition()) return 0;

  // Check Extension (MUST BE BEFORE depth <= 0 check)
  int king_sq = (side_to_move == white) ? get_lsb_index(bitboards[wK]) : get_lsb_index(bitboards[bK]);
  int in_check = is_square_attacked_by(king_sq, side_to_move ^ 1);
  if (in_check) depth++;

  // 3. Drop into Quiescence Search if depth is exhausted
  if (depth <= 0) {
    // Safe guard: If we are STILL in check here (shouldn't happen often with extension),
    // we must not do QS, but force a search to find evasions.
    // However, with depth++ above, we usually ensure we search evasions.
    return quiescence_search(alpha, beta, 0);
  }

  if ((nodes & 2047) == 0) communicate();
  if (stopped) return 0;

  int hashf_flag = hashf_ALPHA;
  int tt_move = 0;
  int val;

  // 4. Transposition Table Probe
  if (ply && ((val = probeTT(alpha, beta, depth)) != NO_TT_ENTRY_FOUND) && !pv_node) {
    return val;
  }

  // Always get the move for sorting
  tt_move = probe_move();

  // Max Ply termination
  if (ply >= MAX_PLY - 1) return eval();

  nodes++;

  // Pre-calculate static eval for pruning
  int static_eval = eval();

  // ===========================
  //       PRUNING LOGIC
  // ===========================

  // 5. Reverse Futility Pruning (Static Null Move)
  if (!pv_node && !in_check && depth <= 8) {
    int margin = 120 * depth;
    if (static_eval - margin >= beta) return static_eval;
  }

  // 6. Null Move Pruning
  U64 has_pieces = ((side_to_move == white)
    ? (bitboards[wN] | bitboards[wB] | bitboards[wR] | bitboards[wQ])
    : (bitboards[bN] | bitboards[bB] | bitboards[bR] | bitboards[bQ]));

  if (!pv_node && depth >= 3 && !in_check && ply && has_pieces) {
    COPY_BOARD();
    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;
    if (en_passant != no_square) hash_key ^= enpassant_keys[en_passant];
    en_passant = no_square;
    side_to_move ^= 1;
    hash_key ^= side_to_move_key;

    int R = 3 + (depth > 6);
    // Reduce depth, but ensure we don't go below 0 instantly causing issues
    int score = -negamax(-beta, -beta + 1, depth - 1 - R);

    ply--; repetition_index--;
    RESTORE_BOARD();
    if (stopped) return 0;
    if (score >= beta) return beta;
  }

  // 7. Futility Pruning
  int f_prune = 0;
  int f_margin[] = { 0, 200, 300, 500 };
  if (!pv_node && !in_check && depth <= 3 &&
    (static_eval + f_margin[depth] <= alpha)) {
    f_prune = 1;
  }

  // ===========================
  //       MOVE SEARCH
  // ===========================

  Moves ml;
  generate_moves(&ml);

  // Sort moves using the TT move
  sort_moves(&ml, tt_move);

  int legal_moves = 0;
  int moves_searched = 0;
  int best_move = 0;

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    // Pruning checks
    int is_capture = get_move_capture_flag(move);
    int is_promo = get_move_promoted_piece(move);

    if (f_prune && legal_moves > 0 && !is_capture && !is_promo) {
      continue;
    }

    COPY_BOARD();
    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (make_move(move, allow_all_moves) == 0) {
      ply--; repetition_index--;
      continue;
    }
    legal_moves++;

    int score;

    if (moves_searched == 0) {
      score = -negamax(-beta, -alpha, depth - 1);
    } else {
      // LMR Logic
      if (moves_searched >= 4 && depth >= 3 && !is_capture && !is_promo && !in_check) {
        int reduction = 1 + (depth / 3) + (moves_searched / 12);
        if (history_moves[get_move_piece(move)][get_move_target(move)] > (depth * 50)) reduction--;
        if (reduction > depth - 1) reduction = depth - 1;
        if (reduction < 1) reduction = 1;

        score = -negamax(-alpha - 1, -alpha, depth - 1 - reduction);
      } else {
        score = alpha + 1;
      }

      if (score > alpha) {
        score = -negamax(-alpha - 1, -alpha, depth - 1);
        if ((score > alpha) && (score < beta)) {
          score = -negamax(-beta, -alpha, depth - 1);
        }
      }
    }

    ply--; repetition_index--;
    RESTORE_BOARD();

    if (stopped) return 0;
    moves_searched++;

    if (score >= beta) {
      storeTT(beta, depth, hashf_BETA, move);
      if (!is_capture && !is_promo) {
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = move;
      }
      return beta;
    }

    if (score > alpha) {
      alpha = score;
      hashf_flag = hashf_EXACT;
      best_move = move;

      if (!is_capture && !is_promo) {
        history_moves[get_move_piece(move)][get_move_target(move)] += depth * depth;
      }

      pv_table[ply][ply] = move;
      for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
      }
      pv_length[ply] = pv_length[ply + 1];
    }
  }

  if (legal_moves == 0) {
    if (in_check) return -MATE_VALUE + ply;
    else return 0;
  }

  storeTT(alpha, depth, hashf_flag, best_move);
  return alpha;
}

// Enhanced search with aspiration windows
void search_position(int depth) {
  ply = 0;
  nodes = 0;
  stopped = 0;
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_moves, 0, sizeof(history_moves));
  memset(pv_table, 0, sizeof(pv_table));
  memset(pv_length, 0, sizeof(pv_length));

  int best_move = 0;

  for (int cur_depth = 1; cur_depth <= depth; cur_depth++) {
    if (stopped == 1) break;

    // Use a small window for search, but fall back to full window if it fails
    // For simplicity and stability, we use full window here
    int score = negamax(NEG_INF, INF, cur_depth);

    if (stopped == 1) break;

    if (pv_length[0] > 0) best_move = pv_table[0][0];

    printf("info depth %d score ", cur_depth);
    if (score > MATE_SCORE)      printf("mate %d ", (MATE_VALUE - score + 1) / 2);
    else if (score < -MATE_SCORE) printf("mate %d ", -(score + MATE_VALUE) / 2);
    else                          printf("cp %d ", score);

    printf("nodes %llu pv ", nodes);
    for (int i = 0; i < pv_length[0]; i++) {
      printf("%s ", get_move_str(pv_table[0][i]));
    }
    printf("\n");
  }

  printf("bestmove ");
  if (best_move) {
    print_move(best_move);
  }
  else {
    // FALLBACK: Find the first LEGAL move
    Moves ml;
    ml.count = 0;
    generate_moves(&ml);

    int found_legal = 0;
    for (int i = 0; i < ml.count; i++) {
        COPY_BOARD();
        if (make_move(ml.moves[i], allow_all_moves)) {
            print_move(ml.moves[i]);
            found_legal = 1;
            RESTORE_BOARD();
            break;
        }
        RESTORE_BOARD();
    }

    if (!found_legal) {
        // Must be checkmate or stalemate, print null or resign?
        // UCI requires a move usually, but if we are mated, we can print (none)
        printf("(none)\n");
    }
  }
}


