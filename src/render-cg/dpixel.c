#include <gint/display.h>

/* dpixel() - change a pixel's color */
void dpixel(int x, int y, int color)
{
	/* Coordinate checks */
	if((uint)x >= 396 || (uint)y >= 224 || color == C_NONE) return;

	gint_vram[396 * y + x] = color;
}
