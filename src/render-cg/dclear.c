#define GINT_NEED_VRAM
#include <gint/display.h>

/* dclear() - fill the screen with a single color */
void dclear(uint16_t color)
{
	/* TOOD: render-cg: dclear(): Consider using the DMA */

	/* Operate in longwords to clear faster. Also start at the end of the
	   VRAM to take advantage of pre-decrement writes. */
	uint32_t *v = (void *)(vram + 88704);
	uint32_t op = (color << 16) | color;

	/* Split the VRAM into 2772 blocks of 16 longwords to reduce the
	   overhead of the loop */
	for(int blocks = 2772; blocks; blocks--)
	{
		*--v = op;
		*--v = op;
		*--v = op;
		*--v = op;

		*--v = op;
		*--v = op;
		*--v = op;
		*--v = op;

		*--v = op;
		*--v = op;
		*--v = op;
		*--v = op;

		*--v = op;
		*--v = op;
		*--v = op;
		*--v = op;
	}
}
