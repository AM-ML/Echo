#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

U64 flipVertical(U64 x) {
/*
			     			   RANK -> 12345678...87654321
			 RANK 1 -> 8, shift  56=7, 12345678 = 1_______ & RANK_8
			 RANK 2 -> 7, shift  40=5, 12345678 = 321_____ & RANK_7
			 RANK 3 -> 6, shift  24=3, 12345678 = 54321___ & RANK_6
			 RANK 4 -> 5, shift   8=1, 12345678 = 7654321_ & RANK_5
			 RANK 5 -> 4, shift  -8=1, 12345678 = _8765432 & RANK_4
			 RANK 6 -> 3, shift -24=3, 12345678 = ___87654 & RANK_3
			 RANK 7 -> 2, shift -40=5, 12345678 = _____876 & RANK_2
			 RANK 8 -> 1, shift -56=7, 12345678 = _______8 & RANK_1
*/

    return  ( (x << 56) ) |
            ( (x << 40) & 0x00ff000000000000 ) |
            ( (x << 24) & 0x0000ff0000000000 ) |
            ( (x <<  8) & 0x000000ff00000000 ) |
            ( (x >>  8) & 0x00000000ff000000 ) |
            ( (x >> 24) & 0x0000000000ff0000 ) |
            ( (x >> 40) & 0x000000000000ff00 ) |
            ( (x >> 56) );
}

void addPiece(U64 *bb, int square)
{
	*bb |= 1ULL << square;
}

void removePiece(U64 *bb, int index)
{
	U64 shifted = 1ULL << index;
	*bb -= shifted;
}

void Shift_UR(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*9) - (1ULL << index);
	*bb += weight;

}

void Shift_UL(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*7) - (1ULL << index);
	*bb += weight;
}

void Shift_DR(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*(-7)) -  (1ULL << index);
	*bb += weight;
}

void Shift_DL(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*(-9)) - (1ULL << index);
	*bb += weight;
}

void Shift_U(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*8) - (1ULL << index);
	*bb += weight;
}

void Shift_L(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*(-1)) - (1ULL << index);
	*bb += weight;
}
void Shift_D(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*(-8)) - (1ULL << index);
	*bb += weight;
}

void Shift_R(U64 *bb, int index, int times)
{
	U64 weight = (1ULL << index+times*1) - (1ULL << index);
	*bb += weight;
}


void PrintBitBoard(U64 bb) {
    U64 shiftMe = 1ULL;

    int rank = 0; // chess board rank index
    int file = 0; // chess board file index
    int sq = 0; // chess board64 square placeholder / index.

    printf("\n");

    for (rank = RANK_8; rank >= RANK_1; rank--) { // Looping from rank 8 to rank 1
        printf("\033[1;96m%d ", rank + 1); // Print the rank number

        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank); // Retrieving the square from board64

            // checks if the bit on that square is = 1, then there is a piece there
            if ((shiftMe << sq) & bb)
                printf("\033[1;91mx ");
            else
                printf("\033[1;93m- ");
        }
        printf("\n"); // Splitting each rank by a newline to make it look like a board
    }
    printf("\033[1;96m  A B C D E F G H\033[0;0m\n\n"); // Printing file labels
}