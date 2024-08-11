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

enum { white, black };


/***** Constants *****/
/****************
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* 0 1 1 1 1 1 1 1
* _______________
* A B C D E F G H
*****************/
const U64 not_A_file = 18374403900871474942ULL;

/*
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
* 1 1 1 1 1 1 1 0
  _______________
  A B C D E F G H
 * */
const U64 not_H_file = 9187201950435737471ULL;
const U64 not_HG_file = 4557430888798830399ULL;
const U64 not_AB_file = 18229723555195321596ULL;
const U64 not_B_file = 18302063728033398269ULL;
const U64 not_G_file = 13816973012072644543ULL;

// set/get/pop macros
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= 1ULL << square)
#define pop_bit(bitboard, square) (get_bit(bitboard, square)? (bitboard -= 1ULL << square): 0)



// prototypes
void print_bitboard(U64 bitboard);





/**** Attacks ****/


/*** Pawns ***/

// pawn attacks table:: [sides][squares]
U64 pawn_attacks[2][64];

// pawn attacks generator function
U64 mask_pawn_attacks(int side, int square) {
  U64 attacks = 0ULL; // attacks bitboard


  U64 bitboard= 0ULL; // piece bitboard
  set_bit(bitboard, square); // set piece on bitboard


  // white side
  if(!side) {
    // if the right pawn attack square is not on A file (not possible)
    if((bitboard >> 7) & not_A_file) attacks |= bitboard >> 7;
    // if the left pawn attack square is not on H file (not possible)
    if((bitboard >> 9) & not_H_file) attacks |= bitboard >> 9;
  }
  // black side
  else {
    if((bitboard << 7) & not_H_file) attacks |= bitboard << 7;
    if((bitboard << 9) & not_A_file) attacks |= bitboard << 9;
  }


  return attacks;
}



/*** Knights ***/
U64 knight_attacks[64];

U64 mask_knight_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if(bitboard << 6 & not_H_file & not_G_file) attacks |= bitboard << 6;
  if(bitboard << 10 & not_A_file & not_B_file) attacks |= bitboard << 10;
  if(bitboard << 15 & not_H_file) attacks |= bitboard << 15;
  if(bitboard << 17 & not_A_file) attacks |= bitboard << 17;

  if(bitboard >> 6 & not_A_file & not_B_file) attacks |= bitboard >> 6;
  if(bitboard >> 10 & not_H_file & not_G_file) attacks |= bitboard >> 10;
  if(bitboard >> 15 & not_A_file) attacks |= bitboard >> 15;
  if(bitboard >> 17 & not_H_file) attacks |= bitboard >> 17;

  return attacks;
}

void init_leaper_attacks() {
  for(int square = 0; square < 64; square++) {
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    knight_attacks[square] = mask_knight_attacks(square);
  }
}


/***** MAIN FUNCTION *****/

int main(void) {
  init_leaper_attacks();

  for(int s = 0; s < 64; s++) {
    print_bitboard(knight_attacks[s]);
  }

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

