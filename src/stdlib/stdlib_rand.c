#include <stdlib.h>

static unsigned int seed = 1;

void srand(unsigned int new_seed)
{
	seed = new_seed;
}

int rand(void)
{
	/* TODO Or maybe seed = (16807 * seed) % ((1 << 31) - 1); */
	seed = seed * 1103515245 + 12345;
	return seed & 0x7fffffff;
}
