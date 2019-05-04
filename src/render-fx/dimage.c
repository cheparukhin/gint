#include <gint/display.h>
#include <display/fx.h>

/* dimage() - render a full image */
void dimage(int x, int y, image_t const *img)
{
	if(img->gray) return;
	bopti_render_clip(x, y, img, 0, 0, img->width, img->height);
}

/* dimage_opt() - render a section of an image */
void dimage_opt(int x, int y, image_t const *img, int left, int top,
	int width, int height, int flags)
{
	if(img->gray) return;

	if(flags & DIMAGE_NOCLIP)
	{
		bopti_render_noclip(x, y, img, left, top, width, height);
	}
	else
	{
		bopti_render_clip(x, y, img, left, top, width, height);
	}
}
