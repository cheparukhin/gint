#include <stdlib.h>
#undef abs
#undef labs

int abs(int x)
{
	return (x < 0) ? (-x) : x;
}

long labs(long x)
{
	return (x < 0) ? (-x) : x;
}
