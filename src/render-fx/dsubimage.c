#include <gint/display.h>
#include <display/fx.h>
#include "bopti-asm.h"

/* List of rendering functions */
static void *bopti_asm[] = {
	bopti_asm_mono,
	bopti_asm_mono_alpha,
};

/* dsubimage(): Render a section of an image */
void dsubimage(int x, int y, bopti_image_t const *img, int left, int top,
	int width, int height, int flags)
{
	DMODE_OVERRIDE(dsubimage, x, y, img, left, top, width, height, flags);

	if(img->gray) return;

	if(flags & DIMAGE_NOCLIP)
	{
		bopti_render_noclip(x, y, img, left, top, width, height,
			gint_vram, NULL, bopti_asm[img->profile]);
	}
	else
	{
		bopti_render_clip(x, y, img, left, top, width, height,
			gint_vram, NULL, bopti_asm[img->profile]);
	}
}
