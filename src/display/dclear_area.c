#include <internals/display.h>
#include <display.h>

/*
	dclear_area()
	Clears an area of the vram using rectangle masks. Both (x1, y1) and
	(x2, y2) are cleared.
*/
void dclear_area(int x1, int y1, int x2, int y2)
{
	uint32_t masks[4];
	if(adjustRectangle(&x1, &y1, &x2, &y2)) return;
	getMasks(x1, x2, masks);

	int begin = y1 << 2;
	int end = (y2 + 1) << 2;
	int i;

	for(i = 0; i < 4; i++) masks[i] = ~masks[i];
	for(i = begin; i < end; i++) vram[i] &= masks[i & 3];
}
