#include <stdio.h>
#include "defs.h"
#include "bitboards.h"
#include <byteswap.h>
#include <time.h>

int main(void)
{
	clock_t start, start1, end, end1;
    double cpu_time_used, cpu_time_used1;

	U64 bb = HEX_DIAGONAL_A1; // A1 DIAGONAL

	start = clock();

	U64 bb_flipped = bswap_64(bb); // v_flipped A1 DIAGONAL -> A8 DIAGONAL

	end = clock();

	start1 = clock();

	U64 bb2_flipped = flipVertical(bb);

	end1 = clock();

	cpu_time_used  = ((double) ( end -  start)) / CLOCKS_PER_SEC * 1000;
	cpu_time_used1 = ((double) (end1 - start1)) / CLOCKS_PER_SEC * 1000;

	printf("\033[1;96mbuilt in bswap: \033[1;93m%lf\033[1;91mau\n", cpu_time_used);
	printf("\033[1;96mcustom bswap:   \033[1;93m%lf\033[1;91mau\n", cpu_time_used);
	// PrintBitBoard(bb);
	// PrintBitBoard(bb_flipped);

	return 0;
}