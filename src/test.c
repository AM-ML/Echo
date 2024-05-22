#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void) {
	U64 bb = 0ULL; addPiece(&bb, A2);

	while(1)
	{
		Shift_R(&bb,A2, 4);
		PrintBitBoard(bb);
		Shift_U(&bb,E2, 4);
		PrintBitBoard(bb);
		Shift_DL(&bb,E6,4);
		PrintBitBoard(bb);
	}
	
    return 0;
}
