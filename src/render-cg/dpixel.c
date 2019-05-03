#define GINT_NEED_VRAM
#include <gint/display.h>

/* dpixel() - change a pixel's color */
void dpixel(int x, int y, uint16_t color)
{
	/* Coordinate checks */
	if((uint)x >= 396 || (uint)y >= 224) return;

	vram[396 * y + x] = color;
}
