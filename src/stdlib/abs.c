#include <stdlib.h>
#undef abs

int abs(int x)
{
	return (x < 0) ? (-x) : (x);
}
