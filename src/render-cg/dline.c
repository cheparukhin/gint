#define GINT_NEED_VRAM
#include <gint/defs/util.h>
#include <gint/display.h>

/* dhline() - optimized drawing of a horizontal line
   @x1 @x2 @y  Coordinates of endpoints of line (both included)
   @color      Any R5G6B5 color */
static void dhline(int x1, int x2, int y, uint16_t color)
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

/* dvline() - optimized drawing of a vertical line
   @y1 @y2 @x  Coordinates of endpoints of line (both included)
   @color      Any R5G6B5 color */
static void dvline(int y1, int y2, int x, uint16_t color)
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

/* dline() - Bresenham line drawing algorithm
   Remotely adapted from MonochromeLib code by Pierre "PerriotLL" Le Gall.
   Relies on dhline() and dvline() for optimized situations.
   @x1 @y1 @x2 @y2  Coordinates of endpoints of line (included)
   @color           Any R5G6B5 color */
void dline(int x1, int y1, int x2, int y2, uint16_t color)
{
	/* Possible optimizations */
	if(y1 == y2)
	{
		dhline(x1, x2, y1, color);
		return;
	}
	if(x1 == x2)
	{
		dvline(y1, y2, x1, color);
		return;
	}

	/* Brensenham line drawing algorithm */

	int i, x = x1, y = y1, cumul;
	int dx = x2 - x1, dy = y2 - y1;
	int sx = sgn(dx), sy = sgn(dy);

	dx = abs(dx), dy = abs(dy);

	dpixel(x1, y1, color);

	if(dx >= dy)
	{
		/* Start with a non-zero cumul to even the overdue between the
		   two ends of the line (for more regularity) */
		cumul = dx >> 1;
		for(i = 1; i < dx; i++)
		{
			x += sx;
			cumul += dy;
			if(cumul > dx) cumul -= dx, y += sy;
			dpixel(x, y, color);
		}
	}
	else
	{
		cumul = dy >> 1;
		for(i = 1; i < dy; i++)
		{
			y += sy;
			cumul += dx;
			if(cumul > dy) cumul -= dy, x += sx;
			dpixel(x, y, color);
		}
	}

	dpixel(x2, y2, color);
}
