#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	U64  bb = (unsigned long long int) 20; // 000..10100 LSB index: 2, MSB index: 4
	printf("LSB:%d\n", bitScanLSB(bb));
	printf("MSB: %d\n", 63 - bitScanMSB(bb));
	return 0;
}