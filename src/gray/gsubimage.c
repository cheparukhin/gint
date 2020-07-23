#include <gint/gray.h>
#include "../render-fx/render-fx.h"
#include "../render-fx/bopti-asm.h"

/* gsubimage(): Render a section of an image */
void gsubimage(int x, int y, bopti_image_t const *img, int left, int top,
	int width, int height, int flags)
{
	uint32_t *light, *dark;
	dgray_getvram(&light, &dark);

	if(flags & DIMAGE_NOCLIP)
	{
		bopti_render_noclip(x, y, img, left, top, width, height,
			light, dark);
	}
	else
	{
		bopti_render_clip(x, y, img, left, top, width, height,
			light, dark);
	}
}
