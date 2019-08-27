#define GINT_NEED_VRAM
#include <gint/display.h>
#include <gint/dma.h>

/* dclear() - fill the screen with a single color */
void dclear(uint16_t color)
{
	dma_memset(vram, (color << 16) | color, 396 * 224 * 2);
	return;
}
