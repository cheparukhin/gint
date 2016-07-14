#include <display_internals.h>
#include <display.h>

/*
	dreverse_area()
	Reverses an area of the vram. This function is a simple application of
	the rectangle masks concept. (x1, y1) and (x2, y2) are reversed as
	well.
*/
void dreverse_area(int x1, int y1, int x2, int y2)
{
	uint32_t masks[4];
	adjustRectangle(&x1, &y1, &x2, &y2);
	getMasks(x1, x2, masks);

	int begin = y1 << 2;
	int end = (y2 + 1) << 2;
	int i;

	for(i = begin; i < end; i++) vram[i] ^= masks[i & 3];
}
