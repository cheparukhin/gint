#include <gint/std/stdlib.h>
#include "tinymt32.h"

static tinymt32_t random;

void srand(unsigned int seed)
{
	tinymt32_init(&random, seed);
}

int rand(void)
{
	return tinymt32_generate_uint32(&random) & 0x7fffffff;
}
