#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

void addPiece(U64 *bb, int square)
{
	*bb |= 1ULL << square;
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