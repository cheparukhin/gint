#define GINT_NEED_VRAM
#include <gint/defs/util.h>
#include <gint/display.h>
#include <display/fx.h>

/* dhline() - optimized drawing of a horizontal line using a rectangle mask
   @x1 @x2 @y  Coordinates of endpoints of line (both included)
   @color      Allowed colors: white, black, none, reverse */
void dhline(int x1, int x2, int y, color_t color)
{
	if((uint)y >= 64) return;
	if(x1 > x2) swap(x1, x2);

	/* Get the masks for the [x1, x2] range */
	uint32_t m[4];
	masks(x1, x2, m);

	uint32_t *data = vram + (y << 2);

	if(color == color_white)
	{
		data[0] &= ~m[0];
		data[1] &= ~m[1];
		data[2] &= ~m[2];
		data[3] &= ~m[3];
	}
	else if(color == color_black)
	{
		data[0] |= m[0];
		data[1] |= m[1];
		data[2] |= m[2];
		data[3] |= m[3];
	}
	else if(color == color_reverse)
	{
		data[0] ^= m[0];
		data[1] ^= m[1];
		data[2] ^= m[2];
		data[3] ^= m[3];
	}
}

/* dvline() - optimized drawing of a vertical line
   This variant is less powerful than dhline() because the line-based structure
   of the vram cannot be used here.
   @y1 @y2 @x  Coordinates of endpoints of line (both included)
   @color      Allowed colors: black, white, none, reverse */
void dvline(int y1, int y2, int x, color_t operator)
{
	if((uint)x >= 128) return;
	if(y1 > y2) swap(y1, y2);

	uint32_t *base = vram + (y1 << 2) + (x >> 5);
	uint32_t *lword = base + ((y2 - y1 + 1) << 4);
	uint32_t mask = 1 << (~x & 31);

	switch(operator)
	{
	case color_white:
		while(lword > base) lword -= 4, *lword &= ~mask;
		break;

	case color_black:
		while(lword > base) lword -= 4, *lword |= mask;
		break;

	case color_reverse:
		while(lword > base) lword -= 4, *lword ^= mask;
		break;

	default:
		break;
	}
}

/* dline() - Bresenham line drawing algorithm
   Remotely adapted from MonochromeLib code by Pierre "PerriotLL" Le Gall.
   Relies on dhline() and dvline() for optimized situations.
   @x1 @y1 @x2 @y2  Coordinates of endpoints of line (included)
   @color           Allowed colors: black, white, none, reverse */
void dline(int x1, int y1, int x2, int y2, color_t color)
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
