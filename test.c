#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64 bb = HEX_DIAGONAL_A1; // A1 DIAGONAL
	U64 bb_flipped = flipVertical(bb); // v_flipped A1 DIAGONAL -> A8 DIAGONAL
	PrintBitBoard(bb);
	PrintBitBoard(bb_flipped);

	return 0;
}