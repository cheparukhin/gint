#include <time.h>
#undef difftime

/*
	clock()
	Should return elapsed CPU time since beginning of program execution.
	This is currently not implemented and returns -1.
*/
clock_t clock(void)
{
	return (clock_t)-1;
}

/*
	difftime()
	Returns the number of seconds between the given points.
*/
double difftime(time_t end, time_t beginning)
{
	return (double)(end - beginning);
}
