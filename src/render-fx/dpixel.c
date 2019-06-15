#define GINT_NEED_VRAM
#include <gint/display.h>
#include <gint/defs/types.h>

/* dpixel() - change a pixel's color */
void dpixel(int x, int y, color_t color)
{
	/* Sanity checks */
	if((uint)x >= 128 || (uint)y >= 64) return;

	uint32_t *lword = vram + (y << 2) + (x >> 5);
	uint32_t mask = 1 << (~x & 31);

	if(color == color_white)
	{
		*lword &= ~mask;
	}
	else if(color == color_black)
	{
		*lword |= mask;
	}
	else if(color == color_invert)
	{
		*lword ^= mask;
	}
}
