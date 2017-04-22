#include <stdlib.h>
#include <stdint.h>

static uint32_t seed = 1;

void srand(unsigned int new_seed)
{
	seed = new_seed;
}

int rand(void)
{
	seed = 16807 * seed;
	uint32_t top = ((uint32_t)1 << 31) - 1;
	while(seed >= top) seed -= top;
	/* TODO Or maybe seed = (16807 * seed) % ((1 << 31) - 1); */
//	seed = seed * 1103515245 + 12345;
	return seed & 0x7fffffff;
}
