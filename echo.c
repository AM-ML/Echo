#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64 bb = 0ULL;

	addPiece(&bb, A2);
	addPiece(&bb, G2);
	addPiece(&bb, E4);

	printf("\033[1;96mAdded pieces on: \033[1;93mA2 G2 E4\n");

	PrintBitBoard(bb);

	return 0;
}