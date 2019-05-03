#define GINT_NEED_VRAM
#include <gint/defs/util.h>
#include <gint/display.h>

/* drect() - fill a rectangle of the screen */
void drect(int x1, int y1, int x2, int y2, uint16_t color)
{
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);

	/* Order and bounds */
	if(x1 >= 396 || x2 < 0 || y1 >= 224 || y2 < 0) return;
	if(x1 < 0) x1 = 0;
	if(x2 >= 396) x2 = 395;
	if(y1 < 0) y1 = 0;
	if(y2 >= 224) y2 = 223;

	/* The method is exactly like dhline(). I first handle odd endpoints,
	   then write longwords for the longest section */

	uint16_t *base = vram + 396 * y1;
	int height = y2 - y1 + 1;

	/* Do borders first if there are at an odd position */

	if(x1 & 1)
	{
		uint16_t *v = base;
		for(int h = height; h; h--)
		{
			v[x1] = color;
			v += 396;
		}
		x1++;
	}

	if(x2 & 1) x2++;
	else
	{
		uint16_t *v = base;
		for(int h = height; h; h--)
		{
			v[x2] = color;
			v += 396;
		}
	}

	/* Now copy everything that's left as longwords */

	uint32_t *v = (void *)(base + x1);
	uint32_t op = (color << 16) | color;
	int width = (x2 - x1) >> 1;

	for(int h = height; h; h--)
	{
		for(int w = 0; w < width; w++) v[w] = op;
		v += 198;
	}
}
