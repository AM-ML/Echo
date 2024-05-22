#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64 bb = 0ULL;

	addPiece(&bb, A2);

	printf("\033[1;91mPiece on: \033[1;93mA2, A6\n");

	addPiece(&bb, A6);
	PrintBitBoard(bb);

	printf("\033[1;91mA2 \033[1;93mTo \033[1;91mE6\n"); //E6
	Shift_UR(&bb, A2, 4);

	PrintBitBoard(bb);

	printf("\033[1;91mNumber of Pieces: \033[1;93m%d\n\n", popCount(bb));


	return 0;
}