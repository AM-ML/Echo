#include "helper.h"
#include "tt.h"

// print bitboard
void print_bitboard(U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8 - rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("1 ");
      }

      else {
        printf("0 ");
      }
    }

    printf("\n");
  }

  printf("    _______________\n");
  printf("    A B C D E F G H\n"); // for navigation

#else
  // print position id
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n", bitboard);

  // loop over ranks / rows
  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank); // for navigation

    // loop over files / columns
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("\033[1;91m1 ");
      }

      else {
        printf("\033[1;96m0 ");
      }
    }

    // seperate ranks
    printf("\n");
  }

  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n"); // for navigation
#endif
}

void print_sides_occupancies() {
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n",
         sides_occupancies[both]);

  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank);
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);
      if (get_bit(sides_occupancies[white], square)) {
        printf("\033[1;94m1 ");
      } else if (get_bit(sides_occupancies[black], square)) {
        printf("\033[1;91m2 ");
      } else {
        printf("\033[1;96m0 ");
      }
    }
    printf("\n");
  }
  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n");
}

void print_bitboard_piece(int piece_square, U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8 - rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (square == piece_square) {
        printf("P ");
      }

      else if (get_bit(bitboard, square)) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }

    printf("\n");
  }

  printf("    _______________\n");
  printf("    A B C D E F G H\n"); // for navigation

#else
  // print position id
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n", bitboard);

  // loop over ranks / rows
  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8 - rank); // for navigation

    // loop over files / columns
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (square == piece_square) {
        printf("\033[1;93mP ");
      }

      else if (get_bit(bitboard, square)) {
        printf("\033[1;91m1 ");
      }

      else {
        printf("\033[1;96m0 ");
      }
    }

    // seperate ranks
    printf("\n");
  }

  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n"); // for navigation
#endif
}

void print_board(int flag) {
  if (FORCE_ASCII) {
    // Windows / ASCII mode
    printf("\nPosition: %llu\n", sides_occupancies[both]);
    printf("Castling: %c%c%c%c\n",
           can_castle & WCK ? 'K' : '_',
           can_castle & WCQ ? 'Q' : '_',
           can_castle & BCK ? 'k' : '_',
           can_castle & BCQ ? 'q' : '_');
    printf("En Passant: %s\n\n",
           en_passant != no_square ? square_to_notation[en_passant] : "_");
    if (side_to_move != -1)
      printf("%s To Move\n", side_to_move == white ? "White" : "Black");
  } else {
    // Colored Unicode mode
    printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m",
           sides_occupancies[both]);
    printf("\n\033[1;93mHash Key: \033[1;95m%llx\033[0;0m",
           hash_key);
    printf("\n\033[1;93mCastling: \033[1;95m%c%c%c%c\033[0;0m",
           can_castle & WCK ? 'K' : '_',
           can_castle & WCQ ? 'Q' : '_',
           can_castle & BCK ? 'k' : '_',
           can_castle & BCQ ? 'q' : '_');
    printf("\n\033[1;93mEn Passant: \033[1;95m%s\033[0;0m\n\n",
           en_passant != no_square ? square_to_notation[en_passant] : "_");
    if (side_to_move != -1)
      printf("%s\033[1;93m To Move\033[0;0m\n",
             side_to_move == white ? "\033[1;96mWhite" : "\033[36mBlack");
  }

  for (int rank = 0; rank < 8; rank++) {
    // top border of row
    printf("   ");
    for (int file = 0; file < 8; file++) printf("+---");
    printf("+\n");

    // rank label
    if (FORCE_ASCII)
      printf(" %d |", 8 - rank);
    else
      printf(" \033[1;93m%d\033[0;0m |", 8 - rank);

    // print pieces
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);
      char *c = " ";   // empty square
      char *color = ""; // default no color

      if (get_bit(sides_occupancies[both], square)) {
        if (!FORCE_ASCII && flag) {
          // Unicode mode
          if (get_bit(bitboards[wP], square))
            c = unicode_pieces[wP], color = "\033[1;96m";
          else if (get_bit(bitboards[wB], square))
            c = unicode_pieces[wB], color = "\033[1;96m";
          else if (get_bit(bitboards[wN], square))
            c = unicode_pieces[wN], color = "\033[1;96m";
          else if (get_bit(bitboards[wR], square))
            c = unicode_pieces[wR], color = "\033[1;96m";
          else if (get_bit(bitboards[wQ], square))
            c = unicode_pieces[wQ], color = "\033[1;96m";
          else if (get_bit(bitboards[wK], square))
            c = unicode_pieces[wK], color = "\033[1;96m";

          else if (get_bit(bitboards[bP], square))
            c = unicode_pieces[bP], color = "\033[36m";
          else if (get_bit(bitboards[bB], square))
            c = unicode_pieces[bB], color = "\033[36m";
          else if (get_bit(bitboards[bN], square))
            c = unicode_pieces[bN], color = "\033[36m";
          else if (get_bit(bitboards[bR], square))
            c = unicode_pieces[bR], color = "\033[36m";
          else if (get_bit(bitboards[bQ], square))
            c = unicode_pieces[bQ], color = "\033[36m";
          else if (get_bit(bitboards[bK], square))
            c = unicode_pieces[bK], color = "\033[36m";
        } else {
          // ASCII mode
          if (get_bit(bitboards[wP], square))
            c = "P";
          else if (get_bit(bitboards[wB], square))
            c = "B";
          else if (get_bit(bitboards[wN], square))
            c = "N";
          else if (get_bit(bitboards[wR], square))
            c = "R";
          else if (get_bit(bitboards[wQ], square))
            c = "Q";
          else if (get_bit(bitboards[wK], square))
            c = "K";
          else if (get_bit(bitboards[bP], square))
            c = "p";
          else if (get_bit(bitboards[bB], square))
            c = "b";
          else if (get_bit(bitboards[bN], square))
            c = "n";
          else if (get_bit(bitboards[bR], square))
            c = "r";
          else if (get_bit(bitboards[bQ], square))
            c = "q";
          else if (get_bit(bitboards[bK], square))
            c = "k";
        }
      }

      // print square
      if (!FORCE_ASCII && flag)
        printf(" %s%s\033[0;0m |", color, c);
      else
        printf(" %s |", c);
    }
    printf("\n");
  }

  // bottom border
  printf("   ");
  for (int file = 0; file < 8; file++) { printf("+---"); }
  printf("+\n");

  // file letters
  if (FORCE_ASCII)
    printf("     A   B   C   D   E   F   G   H\n");
  else
    printf("     \033[1;93mA   B   C   D   E   F   G   H\033[0;0m\n");
}

int bin(int p) {
  int result = 1;
  for (int i = 0; i < p; i++) {
    result *= 2;
  }
  return result;
}
void automate_occupancy(U64 mask) {
  int count = count_bits(mask);

  for (int i = 1, b = 1; i <= count; i++, b = bin(i) - 1) {
    print_bitboard(set_occupancy(b, count, mask));
  }
}


