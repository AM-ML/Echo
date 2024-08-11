#include <stdio.h>

// define bitboard data type
#define U64 unsigned long long

// rank and file to square
#define RF_2SQ(r, f) (r * 8 + f)


// Big Endian File-Rank Mapping
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1
};

// set/get/pop macros
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= 1ULL << square)
#define pop_bit(bitboard, square) (get_bit(bitboard, square)? (bitboard -= 1ULL << square): 0)

// prototypes
void print_bitboard(U64 bitboard);

int main(void) {
  U64 bitboard = 0ULL;

  print_bitboard(bitboard);

  return 0;
}

// print bitboard
void print_bitboard(U64 bitboard) {
  // print position id
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m\n", bitboard);

  // loop over ranks / rows
  for (int rank = 0; rank < 8; rank++) {
    printf("\033[1;93m%d|  \033[0;0m", 8-rank); // for navigation

    // loop over files / columns
    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("\033[1;91m1 ");
      }

      else { printf("\033[1;96m0 "); }
    }

    // seperate ranks
    printf("\n");
  }

  printf("    \033[1;93m_______________\n");
  printf("    A B C D E F G H\033[0;0m\n"); // for navigation
}
