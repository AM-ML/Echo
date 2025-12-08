#pragma once

#ifndef _HELPER_H__
#define _HELPER_H__

#include "board.h"

void print_bitboard(U64 bitboard);
void print_sides_occupancies();
void print_bitboard_piece(int piece_square, U64 bitboard);
void print_board(int flag);

#define print_attacked_squares_by(side)                                        \
  (print_bitboard(get_attacked_squares_by((side))))


int bin(int p); // testing function
void automate_occupancy(U64 mask); // animation function

// --- move output debugging function
#define print_move_info(move)                                                  \
  INFO("Source Square: %s", square_to_notation[get_move_source(move)]);        \
  INFO("Target Square: %s", square_to_notation[get_move_target(move)]);        \
  INFO("Piece: %c", ascii_pieces[get_move_piece(move)]);                       \
  INFO("Promoted Piece: %c%s", ascii_pieces[get_move_promoted_piece(move)],    \
       get_move_promoted_piece(move) == 0 ? " or N/A" : "");                   \
  INFO("Castle: %d", get_move_castling_flag(move) ? 1 : 0);                    \
  INFO("Capture: %d", get_move_capture_flag(move) ? 1 : 0);                    \
  INFO("En Passant: %d", get_move_en_passant_flag(move) ? 1 : 0);              \
  INFO("Double Pawn Push: %d", get_move_double_push_flag(move) ? 1 : 0);


#endif
