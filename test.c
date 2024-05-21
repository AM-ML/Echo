#include <stdio.h>
#include "defs.h"
#include "bitboards.h"
#include <byteswap.h>
#include <time.h>

#define NUM_ITERATIONS 1000000

int main(void) {
    clock_t start, end;
    double cpu_time_used = 0, cpu_time_used1 = 0;

    U64 bb = HEX_DIAGONAL_A1; // A1 DIAGONAL

    // Measure built-in bswap
    start = clock();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        volatile U64 bb_flipped = bswap_64(bb); // Prevent compiler optimization
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000 / NUM_ITERATIONS;

    // Measure custom flipVertical
    start = clock();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        volatile U64 bb2_flipped = flipVertical(bb); // Prevent compiler optimization
    }
    end = clock();
    cpu_time_used1 = ((double) (end - start)) / CLOCKS_PER_SEC * 1000 / NUM_ITERATIONS;

    printf("\033[1;96mbuilt in bswap: \033[1;93m%lf\033[1;91m ms\n", cpu_time_used);
    printf("\033[1;96mcustom bswap:   \033[1;93m%lf\033[1;91m ms\n", cpu_time_used1);

    return 0;
}
