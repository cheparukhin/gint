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

	uint16_t *base = gint_vram + 396 * y1;
	int height = y2 - y1 + 1;

	/* Now copy everything that's left as longwords */

	int ax1 = x1 + (x1 & 1);
	int ax2 = (x2 + 1) & ~1;

	uint32_t *v = (void *)(base + ax1);
	uint32_t op = (color << 16) | color;
	int width = (ax2 - ax1) >> 1;

	for(int h = height; h; h--)
	{
		base[x1] = color;
		base[x2] = color;
		for(int w = 0; w < width; w++) v[w] = op;
		v += 198;
		base += 396;
	}
}
