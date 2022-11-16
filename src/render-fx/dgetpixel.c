#include <gint/display.h>
#include <gint/defs/types.h>
#include "render-fx.h"

int dgetpixel(int x, int y)
{
	if((uint)x >= DWIDTH || (uint)y >= DHEIGHT) return -1;

	DMODE_OVERRIDE(dgetpixel, x, y);

	int offset = (y << 2) + (x >> 5);
	uint32_t mask = 1 << (~x & 31);

	return (gint_vram[offset] & mask) ? C_BLACK : C_WHITE;
}
