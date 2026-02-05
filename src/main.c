#include "tt.h"
#include "uci.h"
#include "eval.h"
#include "board.h"
#include "masks.h"
#include "sliding_masks.h"

void init_default_board_position() {
  bitboards[bP] = 65280ULL;
  bitboards[wP] = 71776119061217280ULL;

  bitboards[wR] = 9295429630892703744ULL;
  bitboards[bR] = 129ULL;

  bitboards[wB] = 2594073385365405696ULL;
  bitboards[bB] = 36ULL;

  bitboards[wN] = 4755801206503243776ULL;
  bitboards[bN] = 66ULL;

  bitboards[wQ] = 576460752303423488ULL;
  bitboards[bQ] = 8ULL;

  bitboards[wK] = 1152921504606846976ULL;
  bitboards[bK] = 16ULL;

  sides_occupancies[black] = 65535ULL;
  sides_occupancies[white] = 18446462598732840960ULL;
  sides_occupancies[both] = 18446462598732906495ULL;
}


void init_all() {
  init_leaper_attacks();
  init_sliding_pieces(bishop);
  init_sliding_pieces(rook);
  init_default_board_position();
  init_hash_keys();
  init_tt();
  init_pawns_eval_masks();
  init_black_pst();
  init_evaluation();
  // init_magic_numbers();
}


int main(void) {
  init_all();

  parse_fen(start_position);
  uci_loop();

  return 0;

}
