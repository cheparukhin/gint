#include <internals/display.h>
#include <display.h>

#define sgn(x)	((x) < 0 ? -1 : 1)
#define	abs(x)	((x) < 0 ? -(x) : (x))
#define	rnd(x)	((int)((x) + 0.5))

/*
	dline()
	Draws a line on the screen. Automatically optimizes horizontal and
	vertical lines.
*/

static void dhline(int x1, int x2, int y, enum Color color)
{
	uint32_t masks[4];
	int offset = y << 2;
	int i;

	// Swapping x1 and x2 if needed.
	if(x1 > x2) x1 ^= x2, x2 ^= x1, x1 ^= x2;
	getMasks(x1, x2, masks);

	switch(color)
	{
	case Color_White:
		for(i = 0; i < 4; i++) vram[offset + i] &= ~masks[i];
		break;

	case Color_Black:
		for(i = 0; i < 4; i++) vram[offset + i] |= masks[i];
		break;

	case Color_Invert:
		for(i = 0; i < 4; i++) vram[offset + i] ^= masks[i];
		break;

	default:
		break;
	}
}

static void dvline(int y1, int y2, int x, enum Color color)
{
	int offset = (y1 << 2) + (x >> 5);
	int end = (y2 << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	switch(color)
	{
	case Color_White:
		while(offset <= end) vram[offset] &= ~mask, offset += 4;
		break;

	case Color_Black:
		while(offset <= end) vram[offset] |= mask, offset += 4;
		break;

	case Color_Invert:
		while(offset <= end) vram[offset] ^= mask, offset += 4;
		break;

	default:
		break;
	}
}

void dline(int x1, int y1, int x2, int y2, enum Color color)
{
	adjustRectangle(&x1, &y1, &x2, &y2);

	// Possible optimizations.
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

	int i, x = x1, y = y1, cumul;
	int dx = x2 - x1, dy = y2 - y1;
	int sx = sgn(dx), sy = sgn(dy);

	dx = abs(dx), dy = abs(dy);

	dpixel(x1, y1, color);

	if(dx >= dy)
	{
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
