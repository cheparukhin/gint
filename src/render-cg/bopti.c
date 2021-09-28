#include <gint/defs/types.h>
#include <gint/display.h>
#include "bopti-asm.h"

struct box {
	/* Target location of top-left corner */
	int x, y;
	/* Width and height of rendered sub-image */
	int w, h;
	/* Source bounding box (low included, high excluded) */
	int left, top;
};

enum {
	PX_R5G6B5  = 0,
	PX_R5G6B5A = 1,
	PX_P8      = 2,
	PX_P4      = 3,
};

void bopti_render(bopti_image_t const *img, struct box *box)
{
	/* Offset in video RAM and output stride */
	uint16_t *target = gint_vram + (396 * box->y + box->x);
	int out = 2 * (396 - box->w);

	int profile = img->profile;
	void const *data = img->data;

	int w = box->w;
	int h = box->h;

	if(profile == PX_R5G6B5 || profile == PX_R5G6B5A)
	{
		/* Input stride */
		int in = 2 * (img->width - w);

		data += 2 * (img->width * box->top + box->left);

		if(profile == PX_R5G6B5)
			bopti_r5g6b5(data, target, w, h, in, out);
		else
			bopti_r5g6b5a(data, target, w, h, in, out, img->alpha);
	}
	else if(profile == PX_P8)
	{
		/* Palette has 0x100 entries of 2 bytes each */
		uint16_t const *palette = data;
		data += 512;

		int in = img->width - w;
		data += img->width * box->top + box->left;

		bopti_p8(data, target, w, h, in, out, palette, img->alpha);
	}
	else if(profile == PX_P4)
	{
		/* Palette has 0x10 entries of 2 bytes each */
		uint16_t const *palette = data;
		data += 32;

		/* Due to nibble alignment being a hassle, in this function the
		   input stride is expressed in pixels. */
		int in = img->width - w + (img->width & 1);
		/* Also we don't move pointers, we just use pixel offsets. */
		int offset = img->width * box->top + box->left;

		bopti_p4(data, target, w, h, in, out, palette, img->alpha,
			offset);
	}
}

void bopti_render_clip(int x, int y, bopti_image_t const *img, int left,
	int top, int width, int height)
{
	/* Adjust the bounding box of the input image */
	if(left < 0) width  += left, x -= left, left = 0;
	if(top  < 0) height += top,  y -= top,  top  = 0;
	if(left + width  > img->width)  width  = img->width  - left;
	if(top  + height > img->height) height = img->height - top;

	/* Check whether the box intersects the screen */
	if(width <= 0 || height <= 0) return;
	if(x + width <= 0 || x >= 396 || y + height <= 0 || y >= 224) return;

	/* Intersect with the bounding box on-screen */
	if(y < 0) top -= y, height += y, y = 0;
	if(y + height > 224) height = (224 - y);
	if(x < 0) left -= x, width += x, x = 0;
	if(x + width > 396) width = (396 - x);

	/* Finish with the renderer */
	struct box box = { x, y, width, height, left, top };
	bopti_render(img, &box);
}

void bopti_render_noclip(int x, int y, bopti_image_t const *img, int left,
	int top, int width, int height)
{
	struct box box = { x, y, width, height, left, top };
	bopti_render(img, &box);
}
