#include <gint/display.h>
#include "render-fx.h"

/* dclear() - fill the screen with a single color */
void dclear(color_t color)
{
	DMODE_OVERRIDE(dclear, color);

	/* SuperH only supports a single write-move addressing mode, which is
	   pre-decrement write; the other similar mode is post-increment
	   read. So we'll use pre-decrement writes to improve performance. */

	if(color != C_WHITE && color != C_BLACK) return;
	uint32_t fill = -(color >> 1);

	uint32_t *index = gint_vram + 256;

	while(index > gint_vram)
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
