#include <stdlib.h>

/*
	div()
	Computes the integer division of numerator by denominator.
*/
div_t div(int numerator, int denominator)
{
	div_t result;

	result.quot = numerator / denominator;
	result.rem = numerator - result.quot * denominator;

	return result;
}

/*
	ldiv()
	Computes the integer division of two long integers.
*/
ldiv_t ldiv(long numerator, long denominator)
{
	ldiv_t result;

	result.quot = numerator / denominator;
	result.rem = numerator - result.quot * denominator;

	return result;
}
