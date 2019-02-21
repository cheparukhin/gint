#define GINT_NEED_VRAM
#include <gint/display.h>

/* dclear() - fill the screen with a single color */
void dclear(color_t color)
{
	/* SuperH only supports a single write-move addressing mode, which is
	   pre-decrement write; the other similar mode is post-increment
	   read. So we'll use pre-decrement writes to improve performance. */

	if(color != color_white && color != color_black) return;
	uint32_t fill = -(color >> 1);

	uint32_t *index = vram + 256;

	while(index > vram)
	{
		/* Do it by batches to avoid losing cycles on loop tests */
		*--index = fill;
		*--index = fill;
		*--index = fill;
		*--index = fill;

		*--index = fill;
		*--index = fill;
		*--index = fill;
		*--index = fill;

		*--index = fill;
		*--index = fill;
		*--index = fill;
		*--index = fill;

		*--index = fill;
		*--index = fill;
		*--index = fill;
		*--index = fill;
	}
}
