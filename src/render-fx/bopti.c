#define GINT_NEED_VRAM
#include <gint/defs/types.h>
#include <gint/display.h>
#include <display/fx.h>
#include "bopti-asm.h"

/* struct rbox: A rendering box (target coordinates and source rectangle)
   Some of the data here is redundant, but makes things easier. */
struct rbox
{
	/* Left pixel of the first column to be drawn, even if this column is
	   not drawn entirely */
	int x;
	/* On-screen location of top-left corner */
	int visual_x, y;
	/* Width of rendered sub-image */
	int width;
	/* Horizontal bounds of the box in the image (included, in columns) */
	int left, right;
	/* Vertical bounds of the box in the image (inc-excluded, in pixels) */
	int top, bottom;
};

/* List of rendering functions */
void *bopti_asm[4] = {
	bopti_asm_mono,		/* asm_mono_t */
	bopti_asm_mono_alpha,	/* asm_mono_t */
	bopti_asm_gray,		/* asm_gray_t */
	bopti_asm_gray_alpha,	/* asm_gray_t */
};

/* struct command: A rendering command
   Includes many computed parameters and handy information. Read-only. */
struct command
{
	/* x-coordinate of rendering box & 31, used for shifts */
	int x;
	/* VRAM pointers */
	uint32_t *v1;
	uint32_t *v2;
	/* Initial offset into VRAM */
	int offset;

	/* Number of VRAM columns affected by the bounding box; this is the
	   same as the number of rendered image columns if x=0, and this number
	   plus 1 otherwise. */
	int columns;
	/* A certain set of rendering masks (see bopti_render()) */
	uint32_t *masks;
	/* Whether the first column is real (ie. x>=0) or not */
	int real_start;

	/* Ignored elements between two rendered grid rows */
	int vram_stride;
	/* Ignored elements between two rendered grid columns */
	int data_stride;

	/* Assembly function, prototype depends on image type */
	union {
		void *asm_void;
		asm_mono_t *asm_mono;
		asm_gray_t *asm_gray;
	};
};

void bopti_grid(void **layer, int rows, int gray, struct command *c)
{
	/* Pointers to vram data */
	uint32_t *v1 = c->v1, *v2 = c->v2;
	/* Current offset into video RAM */
	uint offset = c->offset;
	/* Pairs of VRAM operands. A function that returns such a pair will be
	   optimized by GCC into a function returning into r0,r1 which will
	   avoid some memory accesses. */
	pair_t  p, pret = { 0 };
	/* Same with two pairs for the gray version */
	quadr_t q, qret = { 0 };

	/* Monochrome version */
	if(!gray) while(rows--)
	{
		p.r = pret.r = v1[offset & 0xff];

		for(int col = 0; col < c->columns; col++)
		{
			/* Shift the pair to the left. When x=0, we should have
			   pret.r = p.r but due to some intentional UB with
			   32-bit shifts, pret.r != p.r so we reload p.r. */
			p.l = (c->x) ? pret.r : p.r;
			/* Load new second element, if offset+1 overflows from
			   the VRAM we load from offset 0. It doesn't matter
			   because the result will not be written back, I just
			   want to avoid reading from outside the VRAM. */
			p.r = v1[(offset + 1) & 0xff];

			/* The assembly routine blends a longword of data onto
			   the pair and returns the resulting pair. */
			pret = c->asm_mono(p, layer, c->masks+col+col, -c->x);

			/* Write back the result into VRAM, except for column
			   -1 (occurs once every row, iff visual_x < 0) */
			if(c->real_start + col) v1[offset] = pret.l;

			offset++;
		}

		if(c->x) v1[offset] = pret.r;

		*layer += c->data_stride;
		offset += c->vram_stride;
	}

	/* Gray version */
	else while(rows--)
	{
		if(c->real_start)
		{
			q.r1 = qret.r1 = v1[offset & 0xff];
			q.r2 = qret.r2 = v2[offset & 0xff];
		}

		/* Same as before, but 2 buffers at the same time */
		for(int col = 0; col < c->columns; col++)
		{
			q.l1 = (c->x) ? qret.r1 : q.r1;
			q.r1 = v1[(offset + 1) & 0xff];
			q.l2 = (c->x) ? qret.r2 : q.r2;
			q.r2 = v2[(offset + 1) & 0xff];

			qret = c->asm_gray(q, layer, c->masks+col+col, -c->x);

			if(c->real_start + col)
			{
				v1[offset] = qret.l1;
				v2[offset] = qret.l2;
			}

			offset++;
		}

		if(c->x)
		{
			v1[offset] = qret.r1;
			v2[offset] = qret.r2;
		}

		*layer += c->data_stride;
		offset += c->vram_stride;
	}
}

void bopti_render(image_t const *img, struct rbox *rbox)
{
	/* Compute rendering masks */
	uint32_t vm[4];
	masks(rbox->visual_x, rbox->x + rbox->width - 1, vm);

	/* For each pair of consecutive VRAM elements involved, create a mask
	   from the intersection of the standard vram mask with the shift-mask
	   related to x not being a multiple of 32 */
	uint32_t masks[10] = {
		    0, vm[0],
		vm[0], vm[1],
		vm[1], vm[2],
		vm[2], vm[3],
		vm[3],     0,
	};

	uint32_t mx = 0xffffffff >> (rbox->x & 31);
	for(int i = 0; i < 5; i++)
	{
		masks[2*i]   &= mx;
		masks[2*i+1] &= ~mx;
	}

	/* Position, in masks[], of the first column being rendered */
	int left_origin = (rbox->x >> 5) + 1;

	/* Number of columns in [img] */
	int img_columns = (img->width + 31) >> 5;

	/* Interwoven layer data. Skip left columns that are not rendered */
	const uint32_t *layer = (void *)img->data;
	layer += rbox->top * img_columns;
	layer += rbox->left;

	/* Number of grid columns */
	int columns = rbox->right - rbox->left + 1;

	/* Compute and execute the command for this parameters */
	struct command c = {
	  .x            = rbox->x & 31,
	  /* TODO: bopti: Support gray rendering */
	  .v1           = vram,
	  .v2           = vram,
	  .offset       = (rbox->y << 2) + (rbox->x >> 5),
	  .columns      = columns,
	  .masks        = masks + 2 * left_origin,
	  .real_start   = (left_origin > 0),
	  .vram_stride  = 4 - columns,
	  .data_stride  = (img_columns - columns) << 2,
	  .asm_void     = bopti_asm[img->profile],
	};
	bopti_grid((void **)&layer, rbox->bottom - rbox->top, img->gray, &c);
}

void bopti_render_clip(int visual_x, int y, image_t const *img, int left,
	int top, int width, int height)
{
	/* Left pixel of leftmost column */
	int x = visual_x - (left & 31);
	width += (left & 31);
	left &= ~31;

	/* Adjust the bounding box of the input image */

	if(left < 0) width  += left, x -= left, left = 0;
	if(top  < 0) height += top,  y -= top,  top  = 0;
	if(left + width  > img->width)  width  = img->width  - left;
	if(top  + height > img->height) height = img->height - top;

	/* Check whether the box intersects the screen */
	if(width <= 0 || height <= 0) return;
	if(x + width <= 0 || x > 127 || y + height <= 0 || y > 63) return;

	/* Intersect with the bounding box on-screen. We only need to make sure
	   that x>=-31, not x>=0. Setting x=0 would discard the horizontal
	   alignment information (x & 31). */

	if(y < 0) top -= y, height += y, y = 0;
	if(y + height > 64) height = (64 - y);
	int bottom = top + height;

	if(x < -32)
	{
		int overflow = (x + 32) >> 5;
		overflow = -overflow << 5;
		left  += overflow;
		width -= overflow;
		x += overflow;
	}
	if(x + width > 128) width = (128 - x);
	int right = (left + width - 1) >> 5;
	left >>= 5;

	/* Finish with the standard bopti renderer */
	struct rbox rbox = { x, visual_x, y, width, left, right, top, bottom };
	bopti_render(img, &rbox);
}

void bopti_render_noclip(int visual_x, int y, image_t const *img, int left,
	int top, int width, int height)
{
	/* End row (excluded) */
	int bottom = top + height;

	/* Left pixel of leftmost column */
	int x = visual_x - (left & 31);
	width += (left & 31);

	/* Start column and end column (included) */
	left >>= 5;
	int right = (left + width - 1) >> 5;

	/* Finish with the standard bopti renderer */
	struct rbox rbox = { x, visual_x, y, width, left, right, top, bottom };
	bopti_render(img, &rbox);
}
