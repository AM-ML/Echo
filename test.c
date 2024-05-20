#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64 bb = 0ULL;

	addPiece(&bb, A2);
	addPiece(&bb, G2);
	addPiece(&bb, E4);

	PrintBitBoard(bb);

	return 0;
}