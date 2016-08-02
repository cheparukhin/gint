#include <stdlib.h>

static unsigned int seed = 1;

void srand(unsigned int new_seed)
{
	seed = new_seed;
}

int rand(void)
{
	seed = seed * 1103515245 + 12345;
	return seed & 0x7fffffff;
}
