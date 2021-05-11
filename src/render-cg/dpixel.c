#include <gint/display.h>

/* dpixel() - change a pixel's color */
void dpixel(int x, int y, int color)
{
	/* Coordinate checks */
	if((uint)x >= 396 || (uint)y >= 224 || color == C_NONE) return;
	int index = 396 * y + x;

	if(color == C_INVERT) gint_vram[index] ^= 0xffff;
	else gint_vram[index] = color;
}
