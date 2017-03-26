#include <internals/display.h>
#include <display.h>

/*
	dhline()
	Optimized procedure for drawing an horizontal line. Uses a rectangle
	mask.
*/
static void dhline(size_t x1, size_t x2, int y, color_t operator)
{
	uint32_t masks[4];
	uint32_t *video = vram + (y << 2) + 4;

	// Swapping x1 and x2 if needed.
	if(x1 > x2) x1 ^= x2, x2 ^= x1, x1 ^= x2;
	getMasks(x1, x2, masks);

	switch(operator)
	{
	case color_white:
		*--video &= ~masks[3];
		*--video &= ~masks[2];
		*--video &= ~masks[1];
		*--video &= ~masks[0];
		break;

	case color_black:
		*--video |= masks[3];
		*--video |= masks[2];
		*--video |= masks[1];
		*--video |= masks[0];
		break;

	case color_invert:
		*--video ^= masks[3];
		*--video ^= masks[2];
		*--video ^= masks[1];
		*--video ^= masks[0];
		break;

	default:
		break;
	}
}

/*
	dvline()
	Optimized procedure for drawing a vertical line. This one is far less
	powerful than dhline() because the video ram is essentially line-
	oriented. It also uses a mask.
*/
static void dvline(int y1, int y2, int x, color_t operator)
{
	uint32_t *base	= vram + (y1 << 2) + (x >> 5);
	uint32_t *video	= vram + (y2 << 2) + (x >> 5) + 4;
	uint32_t mask	= 0x80000000 >> (x & 31);

	switch(operator)
	{
	case color_white:
		while(video > base) video -= 4, *video &= ~mask;
		break;

	case color_black:
		while(video > base) video -= 4, *video |= mask;
		break;

	case color_invert:
		while(video > base) video -= 4, *video ^= mask;
		break;

	default:
		break;
	}
}

#define sgn(x)	((x) < 0 ? -1 : 1)
#define	abs(x)	((x) < 0 ? -(x) : (x))

/*
	dline()
	Line drawing algorithm more or less directly taken for MonochromeLib.
	Thanks PierrotLL for this. Relies on dhline() and dvline() for specific
	cases.
*/
void dline(int x1, int y1, int x2, int y2, color_t operator)
{
	if(adjustRectangle(&x1, &y1, &x2, &y2)) return;

	// Possible optimizations.
	if(y1 == y2)
	{
		dhline(x1, x2, y1, operator);
		return;
	}
	if(x1 == x2)
	{
		dvline(y1, y2, x1, operator);
		return;
	}

	int i, x = x1, y = y1, cumul;
	int dx = x2 - x1, dy = y2 - y1;
	int sx = sgn(dx), sy = sgn(dy);

	dx = abs(dx), dy = abs(dy);

	dpixel(x1, y1, operator);

	if(dx >= dy)
	{
		cumul = dx >> 1;
		for(i = 1; i < dx; i++)
		{
			x += sx;
			cumul += dy;
			if(cumul > dx) cumul -= dx, y += sy;
			dpixel(x, y, operator);
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
			dpixel(x, y, operator);
		}
	}

	dpixel(x2, y2, operator);
}
