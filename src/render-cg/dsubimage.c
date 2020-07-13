#include <gint/display.h>
#include "bopti-asm.h"

/* dsubimage(): Render a section of an image */
void dsubimage(int x, int y, bopti_image_t const *img, int left, int top,
	int width, int height, int flags)
{
	if(flags & DIMAGE_NOCLIP)
	{
		bopti_render_noclip(x, y, img, left, top, width, height);
	}
	else
	{
		bopti_render_clip(x, y, img, left, top, width, height);
	}
}
