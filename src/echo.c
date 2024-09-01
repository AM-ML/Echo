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
  a1, b1, c1, d1, e1, f1, g1, h1, no_square
};

enum { white, black, both };
enum { rook, bishop };
enum { wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { WCK = 1, WCQ = 2, BCK = 4, BCQ = 8};

char *unicode_pieces[12] = {"♟︎", "♞", "♝", "♜", "♛", "♚","♙", "♘", "♗", "♖", "♕", "♔"};
char ascii_pieces[12] = "PNBRQKpnbrqk";
int decode_ascii_pieces[] = {
  ['P'] = wP,
  ['N'] = wN,
  ['B'] = wB,
  ['R'] = wR,
  ['Q'] = wQ,
  ['K'] = wK,
  ['p'] = bP,
  ['n'] = bN,
  ['b'] = bB,
  ['r'] = bR,
  ['q'] = bQ,
  ['k'] = bK,
};

U64 bitboards[12]; // pieces bbs
U64 sides_occupancies[3]; // sides

int side_to_move = -1;

int can_castle; // WCK WCQ BCQ BCK

int en_passant = no_square;

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
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= 1ULL << (square))
#define pop_bit(bitboard, square) (get_bit((bitboard), (square))? ((bitboard) -= 1ULL << (square)): 0)
#define count_bits(bitboard) (__builtin_popcountll(bitboard))
#define get_lsb(bitboard) ((bitboard) & -(bitboard))
#define get_tz(bitboard) (((bitboard) & -(bitboard)) - 1)
#define get_lsb_index(bitboard) ((bitboard)? count_bits(get_tz(bitboard)) : -1)


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

void print_bitboard_piece(int piece_square, U64 bitboard) {
#if defined(_WIN32) || defined(_WIN64)
  printf("\nPosition: %llu\n", bitboard);

  for (int rank = 0; rank < 8; rank++) {
    printf("%d|  ", 8-rank);

    for (int file = 0; file < 8; file++) {
      int square = RF_2SQ(rank, file);

      if (square == piece_square) {
        printf("P ");
      }

      else if (get_bit(bitboard, square)) {
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

      if (square == piece_square) {
        printf("\033[1;93mP ");
      }

      else if (get_bit(bitboard, square)) {
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


void print_board(int flag) {

#if defined(_WIN32) || defined(_WIN64)
  flag = 0;
  printf("\nPosition: %llu\n", sides_occupancies[both]);
#else
  printf("\n\033[1;93mPosition: \033[1;95m%llu\033[0;0m", sides_occupancies[both]);
  printf("\n\033[1;93mCastling: \033[1;95m%c%c%c%c\033[0;0m",
                                                              can_castle & WCK? 'K': '_',
                                                              can_castle & WCQ? 'Q': '_',
                                                              can_castle & BCK? 'k': '_',
                                                              can_castle & BCQ? 'q': '_');
  printf("\n\033[1;93mEn Passant: \033[1;95m%s\033[0;0m\n\n", en_passant != no_square? "Available": "_");
  side_to_move != -1 &&  printf("%s\033[1;93m To Move\033[0;0m\n",side_to_move == white? "\033[1;96mWhite" : "\033[1;91mBlack");
#endif


  if (flag) {
    for(int rank = 0; rank < 8; rank++) {
      printf("\033[1;93m%d|  \033[0;0m", 8-rank); // for navigation
      for (int file = 0; file < 8; file++) {
        int square = RF_2SQ(rank, file);
        char *c = ".";
        char *color = "\033[1;91m";

        if (get_bit(sides_occupancies[both], square)) {
          color = "\033[1;96m";

          if (get_bit(bitboards[wP], square)) c = unicode_pieces[wP];
          else if (get_bit(bitboards[wB], square)) c = unicode_pieces[wB];
          else if (get_bit(bitboards[wN], square)) c = unicode_pieces[wN];
          else if (get_bit(bitboards[wR], square)) c = unicode_pieces[wR];
          else if (get_bit(bitboards[wQ], square)) c = unicode_pieces[wQ];
          else if (get_bit(bitboards[wK], square)) c = unicode_pieces[wK];

          else {
            if (get_bit(bitboards[bP], square)) c = unicode_pieces[bP];
            else if (get_bit(bitboards[bB], square)) c = unicode_pieces[bB];
            else if (get_bit(bitboards[bN], square)) c = unicode_pieces[bN];
            else if (get_bit(bitboards[bR], square)) c = unicode_pieces[bR];
            else if (get_bit(bitboards[bQ], square)) c = unicode_pieces[bQ];
            else if (get_bit(bitboards[bK], square)) c = unicode_pieces[bK];
          }

        }

        printf("%s%s ", color, c);
      }
      printf("\n");
    }
    printf("    \033[1;93m_______________\n");
    printf("    A B C D E F G H\033[0;0m\n"); // for navigation
  }
  else {
    for(int rank = 0; rank < 8; rank++) {
      printf("\033[1;93m%d|  \033[0;0m", 8-rank); // for navigation
      for (int file = 0; file < 8; file++) {
        int square = RF_2SQ(rank, file);
        char *c = ".";
        char *color = "\033[1;91m";

        if (get_bit(sides_occupancies[both], square)) {
          color = "\033[1;96m";

          if (get_bit(bitboards[wP], square)) c = "P";
          else if (get_bit(bitboards[wB], square)) c = "B";
          else if (get_bit(bitboards[wN], square)) c = "N";
          else if (get_bit(bitboards[wR], square)) c = "R";
          else if (get_bit(bitboards[wQ], square)) c = "Q";
          else if (get_bit(bitboards[wK], square)) c = "K";

          else {
            if (get_bit(bitboards[bP], square)) c = "p";
            else if (get_bit(bitboards[bB], square)) c = "b";
            else if (get_bit(bitboards[bN], square)) c = "n";
            else if (get_bit(bitboards[bR], square)) c = "r";
            else if (get_bit(bitboards[bQ], square)) c = "q";
            else if (get_bit(bitboards[bK], square)) c = "k";
          }

        }

        printf("%s%s ", color, c);
      }
      printf("\n");
    }
    printf("    \033[1;93m_______________\n");
    printf("    A B C D E F G H\033[0;0m\n"); // for navigation
  }
}


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

U64 bishop_masks[64];
U64 bishop_attacks[64][512]; // 512: max occupancy index for bishops

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
U64 rook_masks[64];
U64 rook_attacks[64][4096]; // 4096: max occupancy index for rooks

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
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);
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

U64 rook_magic_numbers[64] = {
  36046528791461889ULL, 306245049807544320ULL, 36063983539585032ULL, 36046391353016324ULL,
  4647723611581318144ULL, 144119654858752008ULL, 36029346791555584ULL, 144115465103474948ULL,
  2392539453718560ULL, 2305984296994750464ULL, 864831934665064577ULL, 4620834023965460480ULL,
  576602039681811456ULL, 9147945333293184ULL, 562954382901760ULL, 9232519977892855936ULL,
  1188950576505815104ULL, 72198881818984451ULL, 18692788199424ULL, 282574756782088ULL,
  3467915199476925440ULL, 563499776376834ULL, 153126785512374785ULL, 580966551230890244ULL,
  612630426397196288ULL, 70369820020736ULL, 35186527965184ULL, 4611703612769306624ULL,
  9223380835095543936ULL, 562967200407568ULL, 36873226256777220ULL, 2305843292681797761ULL,
  9295500000182141056ULL, 576531189771280384ULL, 576532220844449792ULL, 4644405843593216ULL,
  140754676615170ULL, 576462953482552320ULL, 1168298213896ULL, 619582587905ULL,
  5764748535402627072ULL, 70369281081472ULL, 216190374836732032ULL, 36046389339259008ULL,
  8800389103620ULL, 1125908505198596ULL, 1441153014902816770ULL, 566940008449ULL,
  18014948269498496ULL, 70437465751616ULL, 2377918195974570112ULL, 8798241554560ULL,
  4647723613687120000ULL, 306385529329549440ULL, 288511859718619392ULL, 4574417011200ULL,
  35461397544977ULL, 1407514638827521ULL, 281518196916289ULL, 72620819406652434ULL,
  9223935021436641282ULL, 281483633756161ULL, 1168243820548ULL, 281477128397313ULL
};
U64 bishop_magic_numbers[64] = {
  146369204096991744ULL, 1134698216587296ULL, 9234774526519672896ULL, 73187928636915974ULL,
  9306541899907200ULL, 2401470968299648ULL, 1153203563787190304ULL, 2306478569902129168ULL,
  4708492640384ULL, 11602416136897986688ULL, 9354443096064ULL, 612498483932299264ULL,
  9224502352525000704ULL, 36029355903680512ULL, 2452211101044195328ULL, 72057870056955904ULL,
  580964627081470080ULL, 9007268003645696ULL, 40532431010283528ULL, 45071326708252672ULL,
  4786209012842496ULL, 1143638126102528ULL, 22588371511697408ULL, 288389259944034816ULL,
  4648744022708752ULL, 1143509809762432ULL, 146384891594948672ULL, 2895818958449938464ULL,
  281543712980992ULL, 140874960998400ULL, 283678311747584ULL, 316951423615104ULL,
  4521260533482016ULL, 2256266579951872ULL, 70574970241088ULL, 145273108824320ULL,
  74783970574592ULL, 4611985089885119490ULL, 1169916946948224ULL, 2308101958000771648ULL,
  299102059365889ULL, 74775515897856ULL, 282025823080457ULL, 137841608960ULL,
  8800455114816ULL, 1193612299649155136ULL, 1155175520628900864ULL, 2252900541661696ULL
  , 9298807898891682112ULL, 35751576731648ULL, 1101793591296ULL, 545800192ULL,
  2306265506493038592ULL, 4611757555436257280ULL, 2269410261991424ULL, 1143500689178760ULL,
  73254008854152256ULL, 344805869568ULL, 4303881216ULL, 4611706016336314880ULL,
  21990503096840ULL, 141801095680ULL, 2305847441654022696ULL, 144695738814432384ULL
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

#include <string.h>
U64 find_magic_number(int square, int relevant_bits_count, int flag) {
  U64 occupancies[4096]; // max: 4096 bytes or 12 occupied squares for rook

  U64 attacks[4096]; // max: same as occupied

  U64 used_attacks[4096];

  U64 attack_mask = flag ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  U64 occupancy_indicies = 1 << relevant_bits_count;

  for(int index = 0; index < occupancy_indicies; index++){ // loop over indicies
    occupancies[index] = set_occupancy(index, relevant_bits_count, attack_mask); // store each possibility

    attacks[index] = flag ? relevant_bishop_attacks(square, occupancies[index]) //
                           : relevant_rook_attacks(square, occupancies[index]);
  }

  for(int random_count = 0; random_count < 800000000; random_count++) {
    U64 magic_number = gen_magic_number();

    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

    memset(used_attacks, 0ULL, sizeof(used_attacks));

    int index, fail;

    // loop over occupancy indicies
    for(index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {
      int magic_index = (int)((occupancies[index] * magic_number ) >> (64 - relevant_bits_count));

      if (used_attacks[magic_index] == 0ULL) used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index]) fail = 1;
    }

    if(!fail) return magic_number;

  }
  printf("\b    attempt failed.\n");
  return 0ULL;
}

void init_sliding_pieces(int flag) {
  for(int square = 0; square < 64; square++) {
    int is_bishop = flag == bishop;
    U64 attack_mask = is_bishop? bishop_masks[square] : rook_masks[square];

    int relevant_bits_count = count_bits(attack_mask);

    int occupancy_indicies = 1ULL << relevant_bits_count;

    for(int index = 0; index < occupancy_indicies; index++) {
      if(is_bishop) {
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        int magic_index = (int) ((occupancy * bishop_magic_numbers[square]) >> (64 - relevant_bishop_count_bits[square]));

        bishop_attacks[square][magic_index] = relevant_bishop_attacks(square, occupancy);
      } else {
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        int magic_index = (int) ((occupancy * rook_magic_numbers[square]) >> (64 - relevant_rook_count_bits[square]));

        rook_attacks[square][magic_index] = relevant_rook_attacks(square, occupancy);
      }
    }
  }
}

// get attacks from magic index
// #define get_bishop_attacks(square, blockers) (blockers = (((blockers & bishop_masks[square]) * bishop_magic_numbers[square])) >> (64 - relevant_bishop_count_bits[square]))

// #define get_rook_attacks(square, blockers) (blockers = (((blockers & rook_masks[square]) * rook_magic_numbers[square])) >> (64 - relevant_rook_count_bits[square]))

static inline U64 get_bishop_attacks(int square, U64 blockers) {
  blockers &= bishop_masks[square];
  blockers *= bishop_magic_numbers[square];
  blockers >>= 64 - relevant_bishop_count_bits[square];

  return bishop_attacks[square][blockers];
}

static inline U64 get_rook_attacks(int square, U64 blockers) {
  blockers &= rook_masks[square];
  blockers *= rook_magic_numbers[square];
  blockers >>= 64 - relevant_rook_count_bits[square];

  return rook_attacks[square][blockers];
}

void init_magic_numbers() {
  for(int square = 0; square < 64; square++) {
    rook_magic_numbers[square] = find_magic_number(square, relevant_rook_count_bits[square], rook);
    printf("%lluULL, ", rook_magic_numbers[square]);
  }
  printf("\n");
  for(int square = 0; square < 64; square++) {
    bishop_magic_numbers[square] = find_magic_number(square, relevant_bishop_count_bits[square], bishop);
    printf("%lluULL, ", bishop_magic_numbers[square]);
  }
  printf("\n");
}


int bin(int p) {
  int result = 1;
  for(int i = 0; i < p; i++) {
    result *= 2;
  }
  return result;
}
void automate_occupancy(U64 mask) {
  int count = count_bits(mask);

  for (int i = 1, b = 1; i <= count; i++, b = bin(i) -1) {
    print_bitboard (set_occupancy (b, count, mask));
  }
}

/***** MAIN FUNCTION *****/

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

  sides_occupancies[white] = bitboards[wP] | bitboards[wN] | bitboards[wB] | bitboards[wR] | bitboards[wQ] | bitboards[wK];
  sides_occupancies[black] = bitboards[bP] | bitboards[bN] | bitboards[bB] | bitboards[bR] | bitboards[bQ] | bitboards[bK];
  sides_occupancies[both] = sides_occupancies[white] | sides_occupancies[black];
}

void init_all() {
  init_leaper_attacks();
  init_sliding_pieces(bishop);
  init_sliding_pieces(rook);
  init_default_board_position();
  // init_magic_numbers();
}

int main(void) {
  init_all();

  en_passant = 1;
  side_to_move = black;
  can_castle = 0b1111;
  print_bitboard(sides_occupancies[both]);
  print_board(1);

  for(int piece = wP; piece < bK; piece++) {
    printf("\033[1;93mPiece: \033[1;95m%c\033[0;0m\n", ascii_pieces[piece]);
    print_bitboard(bitboards[piece]);
  }

  return 0;
}

