#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64 bb = 0ULL;

	addPiece(&bb, A2);
	// addPiece(&bb, G2);
	// addPiece(&bb, E4);

	printf("\033[1;96mAdded pieces on: \033[1;93mA2\n");

	PrintBitBoard(bb);

	printf("\033[1;91mPiece on A2 will move diagonally 4 squares:\n"); //E6
	Shift_UR(&bb, A2, 4);

	PrintBitBoard(bb);

	printf("\033[1;91mPiece on E6 return to A2:\n"); //E6
	Shift_DL(&bb, E6, 4);

	PrintBitBoard(bb);

	printf("\033[1;91mPiece on A2 will go Up to A6:\n"); //E6
	Shift_U(&bb, A2, 4);

	PrintBitBoard(bb);

	printf("\033[1;91mPiece on A6 will go diagonally to F1:\n"); //E6
	Shift_DR(&bb, A6, 5);

	PrintBitBoard(bb);


	return 0;
}