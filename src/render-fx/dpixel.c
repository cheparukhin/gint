#include <gint/display.h>
#include <gint/defs/types.h>
#include <display/fx.h>

/* dpixel() - change a pixel's color */
void dpixel(int x, int y, int color)
{
	/* Sanity checks */
	if((uint)x >= 128 || (uint)y >= 64) return;

	DMODE_OVERRIDE(dpixel, x, y, color);

	uint32_t *lword = gint_vram + (y << 2) + (x >> 5);
	uint32_t mask = 1 << (~x & 31);

	if(color == C_WHITE)
	{
		*lword &= ~mask;
	}
	else if(color == C_BLACK)
	{
		*lword |= mask;
	}
	else if(color == C_INVERT)
	{
		*lword ^= mask;
	}
}
