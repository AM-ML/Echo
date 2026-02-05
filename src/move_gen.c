#include "move_gen.h"

// Make move on board
int make_move(int move, int move_flag) {
  if (move_flag == allow_all_moves) {
    COPY_BOARD();

    int source_sqr = get_move_source(move);
    int target_sqr = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted_piece(move);
    int capture_flag = get_move_capture_flag(move);
    int castling_flag = get_move_castling_flag(move);
    int double_push_flag = get_move_double_push_flag(move);
    int en_passant_flag = get_move_en_passant_flag(move);

    // Basic validation
    if (source_sqr < 0 || source_sqr >= 64 || target_sqr < 0 ||
        target_sqr >= 64 || piece < 0 || piece >= 12 ||
        source_sqr == target_sqr) {
      RESTORE_BOARD();
      return 0;
    }
    // Verify piece at source
    if (!get_bit(bitboards[piece], source_sqr)) {
      RESTORE_BOARD();
      return 0;
    }

    // Handle regular captures
    if (capture_flag && !en_passant_flag) {
      int captured_piece = piece_on_squares[target_sqr];
      if (captured_piece == -1) { RESTORE_BOARD(); return 0; }

      hash_key ^= piece_keys[captured_piece][target_sqr];
      pop_bit(bitboards[captured_piece], target_sqr);
    }

    // Move piece
    hash_key ^= piece_keys[piece][source_sqr];
    pop_bit(bitboards[piece], source_sqr);
    piece_on_squares[source_sqr] = -1;

    hash_key ^= piece_keys[piece][target_sqr];
    set_bit(bitboards[piece], target_sqr);
    piece_on_squares[target_sqr] = piece;

    // Handle promotion
    if (promoted_piece) {
        pop_bit(bitboards[(side_to_move == white) ? wP : bP], target_sqr);
        hash_key ^= piece_keys[(side_to_move == white) ? wP : bP][target_sqr];

        set_bit(bitboards[promoted_piece], target_sqr);
        hash_key ^= piece_keys[promoted_piece][target_sqr];
        piece_on_squares[target_sqr] = promoted_piece;
    }

    // Handle en passant capture
    if (en_passant_flag) {
      if (side_to_move == white) {
        pop_bit(bitboards[bP], target_sqr + 8);
        hash_key ^= piece_keys[bP][target_sqr + 8];
        piece_on_squares[target_sqr + 8] = -1;
      } else {
        pop_bit(bitboards[wP], target_sqr - 8);
        hash_key ^= piece_keys[wP][target_sqr - 8];
        piece_on_squares[target_sqr - 8] = -1;
      }
    }

    // Update en passant state
    if (en_passant != no_square) hash_key ^= enpassant_keys[en_passant];
    en_passant = no_square;

    if (double_push_flag) {
      en_passant = (side_to_move == white) ? target_sqr + 8 : target_sqr - 8;
      hash_key ^= enpassant_keys[en_passant];
    }

    // Handle castling
    if (castling_flag) {
      switch (target_sqr) {
        case (g1): // White Kingside
          pop_bit(bitboards[wR], h1); set_bit(bitboards[wR], f1);
          piece_on_squares[h1] = -1; piece_on_squares[f1] = wR;
          hash_key ^= piece_keys[wR][h1]; hash_key ^= piece_keys[wR][f1];
          break;
        case (c1): // White Queenside
          pop_bit(bitboards[wR], a1); set_bit(bitboards[wR], d1);
          piece_on_squares[a1] = -1; piece_on_squares[d1] = wR;
          hash_key ^= piece_keys[wR][a1]; hash_key ^= piece_keys[wR][d1];
          break;
        case (g8): // Black Kingside
          pop_bit(bitboards[bR], h8); set_bit(bitboards[bR], f8);
          piece_on_squares[h8] = -1; piece_on_squares[f8] = bR;
          hash_key ^= piece_keys[bR][h8]; hash_key ^= piece_keys[bR][f8];
          break;
        case (c8): // Black Queenside
          pop_bit(bitboards[bR], a8); set_bit(bitboards[bR], d8);
          piece_on_squares[a8] = -1; piece_on_squares[d8] = bR;
          hash_key ^= piece_keys[bR][a8]; hash_key ^= piece_keys[bR][d8];
          break;
      }
    }

    // Update castling rights
    hash_key ^= castle_keys[can_castle];
    can_castle &= castling_rights[source_sqr];
    can_castle &= castling_rights[target_sqr];
    hash_key ^= castle_keys[can_castle];

    set_sides_occupancies();

    side_to_move ^= 1;
    hash_key ^= side_to_move_key;

    // Verify king safety
    int king_sq = get_lsb_index(bitboards[(side_to_move == white) ? bK : wK]);

    if (is_square_attacked_by(king_sq, side_to_move)) {
      RESTORE_BOARD();
      return 0;
    } else {
      return 1;
    }
  }
  else {
    // Handle capture-only flag
    if (get_move_capture_flag(move)) return make_move(move, allow_all_moves);
    else return 0;
  }
}



void generate_moves(Moves *moves_list) {
  moves_list->count = 0;
  int src_sqr, dest_sqr;
  U64 position, attacks;

  int base = side_to_move == white? wP : bP;
  for (int i = 0; i < 6; i++) {
    int piece = base + i;
    position = bitboards[piece];

    if (side_to_move == white) {
      if (piece == wP) {
        while (position) {
          src_sqr = get_lsb_index(position);
          dest_sqr = src_sqr - 8;

          if (!(dest_sqr < a8) && !get_bit(sides_occupancies[both], dest_sqr)) {
            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 0, 0, 0, 0));
            } else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 0, 0, 0, 0));

              if (src_sqr >= a2 && src_sqr <= h2 &&
                  !get_bit(sides_occupancies[both], dest_sqr - 8)) {
                add_move(moves_list,
                         encode_move(src_sqr, dest_sqr - 8, wP, 0, 0, 1, 0, 0));
              }
            }
          }

          attacks = pawn_attacks[white][src_sqr] & sides_occupancies[black];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0 || dest_sqr >= no_square) {
              pop_bit(attacks, dest_sqr);
              continue;
            }

            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 1, 0, 0, 0));
            }

            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[white][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[bP], en_passant + 8)) {

              add_move(moves_list,
                       encode_move(src_sqr, en_passant, wP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }

      if (piece == wK) {

        if (can_castle &
            WCK) {
          if (!get_bit(sides_occupancies[both], f1) &&
              !get_bit(sides_occupancies[both], g1)) {
            if (!is_square_attacked_by(e1, black) &&
                !is_square_attacked_by(f1, black) &&
                !is_square_attacked_by(g1, black)) {
              add_move(moves_list, encode_move(e1, g1, wK, 0, 0, 0, 0, 1));
            }
          }
        }

        if (can_castle & WCQ) {
          if (!get_bit(sides_occupancies[both], d1) &&
              !get_bit(sides_occupancies[both], c1) &&
              !get_bit(sides_occupancies[both], b1)) {
            if (!is_square_attacked_by(e1, black) &&
                !is_square_attacked_by(d1, black) &&
                !is_square_attacked_by(c1, black)) {
              add_move(moves_list, encode_move(e1, c1, wK, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    } else {
      if (piece == bP) {
        while (position) {
          src_sqr = get_lsb_index(position);
          dest_sqr = src_sqr + 8;

          if (!(dest_sqr > h1) && !get_bit(sides_occupancies[both], dest_sqr)) {
            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 0, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 0, 0, 0, 0));
            } else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 0, 0, 0, 0));

              if (src_sqr >= a7 && src_sqr <= h7 &&
                  !get_bit(sides_occupancies[both], dest_sqr + 8)) {
                add_move(moves_list,
                         encode_move(src_sqr, dest_sqr + 8, bP, 0, 0, 1, 0, 0));
              }
            }
          }
          attacks = pawn_attacks[black][src_sqr] & sides_occupancies[white];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0 || dest_sqr >= no_square) {
              pop_bit(attacks, dest_sqr);
              continue;
            }

            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 1, 0, 0, 0));
            }

            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }
          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[black][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[wP], en_passant - 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, bP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
      if (piece == bK) {

        if (can_castle &
            BCK) {
          if (!get_bit(sides_occupancies[both], f8) &&
              !get_bit(sides_occupancies[both], g8)) {
            if (!is_square_attacked_by(e8, white) &&
                !is_square_attacked_by(f8, white) &&
                !is_square_attacked_by(g8, white)) {
              add_move(moves_list, encode_move(e8, g8, bK, 0, 0, 0, 0, 1));
            }
          }
        }

        if (can_castle & BCQ) {
          if (!get_bit(sides_occupancies[both], d8) &&
              !get_bit(sides_occupancies[both], c8) &&
              !get_bit(sides_occupancies[both], b8)) {
            if (!is_square_attacked_by(e8, white) &&
                !is_square_attacked_by(d8, white) &&
                !is_square_attacked_by(c8, white)) {
              add_move(moves_list, encode_move(e8, c8, bK, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    if ((side_to_move == white) ? piece == wN : piece == bN) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = knight_attacks[src_sqr] & ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wB : piece == bB) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_bishop_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wR : piece == bR) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_rook_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wQ : piece == bQ) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = get_queen_attacks(src_sqr, sides_occupancies[both]) &
                  ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wK : piece == bK) {
      while (position) {
        src_sqr = get_lsb_index(position);
        attacks = king_attacks[src_sqr] & ~sides_occupancies[side_to_move];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          // FIX: Proper bounds checking
          if (dest_sqr < 0 || dest_sqr >= no_square) {
            pop_bit(attacks, dest_sqr);
            continue;
          }

          if (!get_bit(sides_occupancies[side_to_move == white ? black : white],
                       dest_sqr)) {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 0, 0, 0, 0));
          } else {
            add_move(moves_list,
                     encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          }
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }
  }
}

// Generate capture moves
void generate_capture_moves(Moves *moves_list) {
  moves_list->count = 0;
  int src_sqr, dest_sqr;
  U64 position, attacks;

  int base = side_to_move == white? wP : bP;
  for (int i = 0; i < 6; i++) {
    int piece = base + i;
    position = bitboards[piece];

    if (side_to_move == white) {
      if (piece == wP) {
        while (position) {
          src_sqr = get_lsb_index(position);
          attacks = pawn_attacks[white][src_sqr] & sides_occupancies[black];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0) continue;
            if (src_sqr >= a7 && src_sqr <= h7) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, wN, 1, 0, 0, 0));
            }
            // regular pawn capture move
            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, wP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[white][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[bP], en_passant + 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, wP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
    } else {
      if (piece == bP) {
        while (position) {
          src_sqr = get_lsb_index(position);

          // Only pawn captures (no quiet moves)
          attacks = pawn_attacks[black][src_sqr] & sides_occupancies[white];
          while (attacks) {
            dest_sqr = get_lsb_index(attacks);
            if (dest_sqr < 0) { /* shouldn't happen because while(attacks) guards it */ continue; }
            // pawn capture promotion move
            if (src_sqr >= a2 && src_sqr <= h2) {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bQ, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bR, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bB, 1, 0, 0, 0));
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, bN, 1, 0, 0, 0));
            }
            // regular pawn capture move
            else {
              add_move(moves_list,
                       encode_move(src_sqr, dest_sqr, bP, 0, 1, 0, 0, 0));
            }
            pop_bit(attacks, dest_sqr);
          }

          // en passant capture
          if (en_passant != no_square) {
            U64 can_en_passant =
                pawn_attacks[black][src_sqr] & (1ULL << en_passant);

            if (can_en_passant && get_bit(bitboards[wP], en_passant - 8)) {
              add_move(moves_list,
                       encode_move(src_sqr, en_passant, bP, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(position, src_sqr);
        }
      }
    }

    if ((side_to_move == white) ? piece == wN : piece == bN) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = knight_attacks[src_sqr] & sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wB : piece == bB) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_bishop_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wR : piece == bR) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_rook_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wQ : piece == bQ) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = get_queen_attacks(src_sqr, sides_occupancies[both]) &
                  sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }

    if ((side_to_move == white) ? piece == wK : piece == bK) {
      while (position) {
        src_sqr = get_lsb_index(position);
        // Only attacks that hit enemy pieces (captures)
        attacks = king_attacks[src_sqr] & sides_occupancies[side_to_move == white ? black : white];
        while (attacks) {
          dest_sqr = get_lsb_index(attacks);
          add_move(moves_list,
                   encode_move(src_sqr, dest_sqr, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, dest_sqr);
        }
        pop_bit(position, src_sqr);
      }
    }
  }
}
