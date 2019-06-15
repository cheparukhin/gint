#define GINT_NEED_VRAM
#include <gint/display.h>
#include <gint/defs/util.h>

/* dhline() - optimized drawing of a horizontal line */
void dhline(int x1, int x2, int y, uint16_t color)
{
	/* Order and bounds */
	if((uint)y >= 224) return;
	if(x1 > x2) swap(x1, x2);
	if(x1 < 0) x1 = 0;
	if(x2 >= 396) x2 = 395;

	int offset = 396 * y;

	/* Use longwords to do the copy, but first paint the endpoints to heed
	   for odd x1 and x2. Checking the parity may be a waste of time. */
	vram[offset + x1] = color;
	vram[offset + x2] = color;

	/* Now round to longword boundaries and copy everything in-between with
	   longwords */
	x1 = x1 + (x1 & 1);
	x2 = (x2 + 1) & ~1;

	uint32_t *start = (void *)(vram + offset + x1);
	uint32_t *end   = (void *)(vram + offset + x2);
	uint32_t op = (color << 16) | color;

	while(end > start) *--end = op;
}

/* dvline() - optimized drawing of a vertical line */
void dvline(int y1, int y2, int x, uint16_t color)
{
	/* Order and bounds */
	if((uint)x >= 395) return;
	if(y1 > y2) swap(y1, y2);
	if(y1 < 0) y1 = 0;
	if(y2 >= 224) y2 = 223;

	uint16_t *v = vram + 396 * y1 + x;
	int height = y2 - y1 + 1;

	while(height--) *v = color, v += 396;
}

