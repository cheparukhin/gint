#include <gint/gray.h>
#include <display/fx.h>
#include "../render-fx/bopti-asm.h"

/* List of rendering functions */
static void *bopti_asm[] = {
	bopti_gasm_mono,
	bopti_gasm_mono_alpha,
	bopti_gasm_gray,
	bopti_gasm_gray_alpha,
};

/* gimage(): Render a full image */
void gimage(int x, int y, image_t const *img)
{
	uint32_t *light, *dark;
	gvram(&light, &dark);

	bopti_render_clip(x, y, img, 0, 0, img->width, img->height, light,
		dark, bopti_asm[img->profile]);
}

/* gsubimage(): Render a section of an image */
void gsubimage(int x, int y, image_t const *img, int left, int top,
	int width, int height, int flags)
{
	uint32_t *light, *dark;
	gvram(&light, &dark);

	if(flags & DIMAGE_NOCLIP)
	{
		bopti_render_noclip(x, y, img, left, top, width, height,
			light, dark, bopti_asm[img->profile]);
	}
	else
	{
		bopti_render_clip(x, y, img, left, top, width, height,
			light, dark, bopti_asm[img->profile]);
	}
}
