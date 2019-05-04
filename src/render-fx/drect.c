#define GINT_NEED_VRAM
#include <gint/defs/util.h>
#include <gint/display.h>
#include <display/fx.h>

/* drect() - fill a rectangle of the screen */
void drect(int x1, int y1, int x2, int y2, color_t color)
{
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);

	/* Argument checking */
	if(x1 >= 128 || x2 < 0 || y1 >= 64 || y2 < 0) return;
	if(x1 < 0) x1 = 0;
	if(x2 >= 128) x2 = 127;
	if(y1 < 0) y1 = 0;
	if(y2 >= 64) y2 = 63;

	/* Use masks to get the work done fast! */
	uint32_t m[4];
	masks(x1, x2, m);

	uint32_t *base	= vram + (y1 << 2);
	uint32_t *lword	= vram + (y2 << 2) + 4;

	if(color == color_white) while(lword > base)
	{
		*--lword &= ~m[3];
		*--lword &= ~m[2];
		*--lword &= ~m[1];
		*--lword &= ~m[0];
	}
	else if(color == color_black) while(lword > base)
	{
		*--lword |= m[3];
		*--lword |= m[2];
		*--lword |= m[1];
		*--lword |= m[0];
	}
	else if(color == color_reverse) while(lword > base)
	{
		*--lword ^= m[3];
		*--lword ^= m[2];
		*--lword ^= m[1];
		*--lword ^= m[0];
	}
}
