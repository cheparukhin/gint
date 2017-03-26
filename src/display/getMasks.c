#include <display.h>

/*
	getMasks()
	Computes the rectangle masks needed to affect pixels located between x1
	and x2 (both included). The four masks are stored in the third argument
	(seen as an array).
*/
void getMasks(size_t x1, size_t x2, uint32_t *masks)
{
	// Indexes of the first and last longs that are not empty.
	size_t l1 = x1 >> 5;
	size_t l2 = x2 >> 5;
	size_t i = 0;

	// Setting the base masks. Those are the final values, except for the
	// longs between indexes l1 and l2, that still need to be adjusted.
	while(i < l1)	masks[i++] = 0x00000000;
	while(i <= l2)	masks[i++] = 0xffffffff;
	while(i < 4)	masks[i++] = 0x00000000;

	// Removing the long number information in x1 and x2 (that is, the
	// multiples of 32) to keep only the interesting information -- the
	// number of null bits to add in l1 and l2.
	x1 &= 31;
	// Inverting x2 is here the same as computing 32 - x, since 32 is a
	// power of 2 (positive bits at the left are removed by the mask).
	x2 = ~x2 & 31;

	// Setting the first and last masks.
	masks[l1] &= (0xffffffff >> x1);
	masks[l2] &= (0xffffffff << x2);
}
