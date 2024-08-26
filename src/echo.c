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
const char *index_to_notation[] = {
  "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
  "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
  "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
  "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
  "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
  "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
  "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
  "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1"
};


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
const U64 not_rank_1 = 72057594037927935ULL;
const U64 not_rank_8 = 18446744073709551360ULL;

// set/get/pop macros
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= 1ULL << square)
#define pop_bit(bitboard, square) (get_bit(bitboard, square)? (bitboard -= 1ULL << square): 0)
#define count_bits(bitboard) (__builtin_popcountll(bitboard))
#define get_lsb(bitboard) ((bitboard) & -(bitboard))
#define get_tz(bitboard) (((bitboard) & -(bitboard)) - 1)
#define get_lsb_index(bitboard) ((bitboard)? count_bits(get_tz(bitboard)) : -1)


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

  if(bitboard << 6 & not_HG_file) attacks |= bitboard << 6;
  if(bitboard << 10 & not_AB_file) attacks |= bitboard << 10;
  if(bitboard << 15 & not_H_file) attacks |= bitboard << 15;
  if(bitboard << 17 & not_A_file) attacks |= bitboard << 17;

  if(bitboard >> 6 & not_AB_file) attacks |= bitboard >> 6;
  if(bitboard >> 10 & not_HG_file) attacks |= bitboard >> 10;
  if(bitboard >> 15 & not_A_file) attacks |= bitboard >> 15;
  if(bitboard >> 17 & not_H_file) attacks |= bitboard >> 17;

  return attacks;
}


/*** King ***/
U64 king_attacks[64];

U64 mask_king_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if((bitboard << 8)) attacks |= bitboard << 8;
  if((bitboard << 9) & not_A_file) attacks |= bitboard << 9;
  if((bitboard << 7) & not_H_file) attacks |= bitboard << 7;
  if((bitboard << 1) & not_A_file) attacks |= bitboard << 1;

  if((bitboard >> 8)) attacks |= bitboard >> 8;
  if((bitboard >> 9) & not_H_file) attacks |= bitboard >> 9;
  if((bitboard >> 7) & not_A_file) attacks |= bitboard >> 7;
  if((bitboard >> 1) & not_H_file) attacks |= bitboard >> 1;

  return attacks;
}


/**** bishop ****/

U64 bishop_attacks[64];

U64 mask_bishop_attacks(int square) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  // mask relevant bishop occupancy bits
  for(r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++) attacks |= (1ULL << (RF_2SQ(r, f)));
  for(r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--) attacks |= (1ULL << (RF_2SQ(r, f)));
  for(r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--) attacks |= (1ULL << (RF_2SQ(r, f)));
  for(r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++) attacks |= (1ULL << (RF_2SQ(r, f)));

  return attacks;
}

U64 relevant_bishop_attacks(int square, U64 block) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  // mask relevant bishop occupancy bits + board edge
  for(r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
    attacks |= (1ULL << (RF_2SQ(r, f))); // add attack square
    if ((1ULL << (RF_2SQ(r, f))) & block) break; // then break, indicate piece can be captured
  }

  for(r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block) break;
  }

  for(r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block) break;
  }

  for(r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
    attacks |= (1ULL << (RF_2SQ(r, f)));
    if ((1ULL << (RF_2SQ(r, f))) & block) break;
  }

  return attacks;
}


/**** rook ****/
U64 rook_attacks[64];

U64 mask_rook_attacks(int square) {
  U64 attacks = 0ULL;

  int r, f;
  int tr, tf;
  tr = square / 8;
  tf = square % 8;


  for (r = tr+1; r < 7; r++) attacks |= (1ULL << (RF_2SQ(r, tf)));
  for (r = tr-1; r > 0; r--) attacks |= (1ULL << (RF_2SQ(r, tf)));
  for (f = tf+1; f < 7; f++) attacks |= (1ULL << (RF_2SQ(tr, f)));
  for (f = tf-1; f > 0; f--) attacks |= (1ULL << (RF_2SQ(tr, f)));

  return attacks;
}

U64 relevant_rook_attacks(int square, U64 block) {
  U64 attacks = 0ULL;

  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // init rank, file
  int r, f;

  // init target rank, file
  int tr, tf;
  tr = square / 8;
  tf = square % 8;

  for (r = tr+1; r <= 7; r++) {
    attacks |= (1ULL << (RF_2SQ(r, tf)));
    if ((1ULL << (RF_2SQ(r, tf))) & block) break;
  }

  for (r = tr-1; r >= 0; r--) {
    attacks |= (1ULL << (RF_2SQ(r, tf)));
    if ((1ULL << (RF_2SQ(r, tf))) & block) break;
  }

  for (f = tf+1; f <= 7; f++) {
    attacks |= (1ULL << (RF_2SQ(tr, f)));
    if ((1ULL << (RF_2SQ(tr, f))) & block) break;
  }

  for (f = tf-1; f >= 0; f--) {
    attacks |= (1ULL << (RF_2SQ(tr, f)));
    if ((1ULL << (RF_2SQ(tr, f))) & block) break;
  }


  return attacks;
}

void init_leaper_attacks() {
  for(int square = 0; square < 64; square++) {
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    knight_attacks[square] = mask_knight_attacks(square);
    king_attacks[square] = mask_king_attacks(square);
    bishop_attacks[square] = mask_bishop_attacks(square);
    rook_attacks[square] = mask_rook_attacks(square);
  }
}


/**** RELEVANT BIT COUNT LOOKUP TABLE ****/
const int relevant_bishop_count_bits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

const int relevant_rook_count_bits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const int relevant_knight_count_bits[64] = {
  2, 3, 4, 4, 4, 4, 3, 2,
  3, 4, 6, 6, 6, 6, 4, 3,
  4, 6, 8, 8, 8, 8, 6, 4,
  4, 6, 8, 8, 8, 8, 6, 4,
  4, 6, 8, 8, 8, 8, 6, 4,
  4, 6, 8, 8, 8, 8, 6, 4,
  3, 4, 6, 6, 6, 6, 4, 3,
  2, 3, 4, 4, 4, 4, 3, 2
};

const int relevant_queen_count_bits[64] = {
  18, 16, 16, 16, 16, 16, 16, 18,
  16, 15, 15, 15, 15, 15, 15, 16,
  16, 15, 17, 17, 17, 17, 15, 16,
  16, 15, 17, 19, 19, 17, 15, 16,
  16, 15, 17, 19, 19, 17, 15, 16,
  16, 15, 17, 17, 17, 17, 15, 16,
  16, 15, 15, 15, 15, 15, 15, 16,
  18, 16, 16, 16, 16, 16, 16, 18
};

/**** OCCUPANCY AND MAGIC SECTION ****/

// creates attack mask and maps each occupied square to a bit
// i.e: 1 = first occupied square from top left to bottom right
// 2 = 2nd, 3 = 1st + 2nd, 4 = 3rd, 5 = 3rd + 1st, 6 = 3rd + 2nd + 1st etc..
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; count++) {
    int square = get_lsb_index(attack_mask); // from top left to bottom right
    pop_bit(attack_mask, square);

    if ((U64) index & (1ULL << count)) { // if square in specified bit array
      set_bit(occupancy, square); // add bit
    }
  }

  return occupancy;
}


/**** pseudo random number state ****/
unsigned int state = 1804289383;

unsigned int get_random_32() {
  unsigned int number = state;
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  state = number;

  return number;
}

U64 get_random_64() {
  U64 n1, n2, n3, n4;

  n1 = (U64) (get_random_32()) & 0XFFFF; // slice 16 bits from MS1B side
  n2 = (U64) (get_random_32()) & 0XFFFF;
  n3 = (U64) (get_random_32()) & 0XFFFF;
  n4 = (U64) (get_random_32()) & 0XFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 gen_magic_number() {
  return get_random_64() & get_random_64() & get_random_64() & get_random_64();
}

// print bitboard
void print_bitboard(U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8-rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (get_bit(bitboard, square)) {
        printf("1 ");
      }

      else { printf("0 "); }
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
  #endif
}


/***** MAIN FUNCTION *****/

int main(void) {
  init_leaper_attacks();

    print_bitboard(gen_magic_number());
    print_bitboard(gen_magic_number());
    print_bitboard(gen_magic_number());

  return 0;
}

