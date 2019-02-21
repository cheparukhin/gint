#include <display/fx.h>

/* masks() - compute the vram masks for a given rectangle */
void masks(int x1, int x2, uint32_t *masks)
{
	if(x1 < 0) x1 = 0;
	if(x2 < 0) x2 = 0;

	/* Indexes of the first and last non-empty longs */
	size_t l1 = x1 >> 5;
	size_t l2 = x2 >> 5;
	size_t i = 0;

	/* Base masks (0's are final, 0xffffffff will be adjusted later) */
	while(i < l1)	masks[i++] = 0x00000000;
	while(i <= l2)	masks[i++] = 0xffffffff;
	while(i < 4)	masks[i++] = 0x00000000;

	/* Remove the index information in x1 and x2 (it's now in l1 and l2)
	   and keep only the offsets */
	x1 &= 31;
	/* For x2 we also want the complement to 31 to invert the shift */
	x2 = ~x2 & 31;

	/* Now roll! */
	masks[l1] &= (0xffffffffu >> x1);
	masks[l2] &= (0xffffffffu << x2);
}
