#include <stdio.h>
#include "defs.h"
#include "bitboards.h"

int main(void)
{
	unsigned long long b64 = 3; // 000...011 number of bits = 1: 2
	printf("Pop Count of 011: %d\n", popCount(b64));
	
	return 0;
}