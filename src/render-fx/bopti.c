#include <gint/defs/types.h>
#include <gint/display.h>
#include "render-fx.h"
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
	int top, height;
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

	/* Whether the image should be drawn on gray mode (this may be 1 even
	   for images of the mono and mono_alpha profiles) */
	int gray;

	/* Assembly function, prototype depends on image type */
	bopti_asm_t f;
};


/* List of rendering functions */
static asm_mono_t *asm_mono[] = {
	bopti_asm_mono,
	bopti_asm_mono_alpha,
};
static asm_gray_t *asm_gray[] = {
	bopti_gasm_mono,
	bopti_gasm_mono_alpha,
	bopti_gasm_gray,
	bopti_gasm_gray_alpha,
};
static asm_mono_scsp_t *asm_mono_scsp[] = {
	bopti_asm_mono_scsp,
	bopti_asm_mono_alpha_scsp,
};
static asm_gray_scsp_t *asm_gray_scsp[] = {
	bopti_gasm_mono_scsp,
	bopti_gasm_mono_alpha_scsp,
	bopti_gasm_gray_scsp,
	bopti_gasm_gray_alpha_scsp,
};

void bopti_grid(void **layer, int rows, struct command *c)
{
	/* Pointers to vram data */
	uint32_t *v1 = c->v1, *v2 = c->v2;
	/* Current offset into video RAM */
	uint offset = c->offset;
	/* Pairs of VRAM operands. A function that returns such a pair will be
	   optimized by GCC into a function returning into r0,r1 which will
	   avoid some memory accesses. */
	pair_t  p, pret = { 0 };
	/* Same with two pairs for the gray version (no optimization here) */
	quadr_t q, qret = { 0 };

	/* Monochrome version */
	if(!c->gray) while(rows--)
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
			pret = c->f.asm_mono(p, layer, c->masks+col+col,-c->x);

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

			c->f.asm_gray(q, layer, c->masks+col+col, -c->x,&qret);

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

void bopti_render(bopti_image_t const *img, struct rbox *rbox, uint32_t *v1,
	uint32_t *v2)
{
	/* Rendering function */
	bopti_asm_t f;
	if(v2) f.asm_gray = asm_gray[img->profile];
	else   f.asm_mono = asm_mono[img->profile];

	/* Compute rendering masks */
	uint32_t vm[4];
	masks(rbox->visual_x, rbox->visual_x + rbox->width - 1, vm);

	/* Number of layers per profile */
	int layer_count[] = { 1, 2, 2, 3 };
	int layers = layer_count[img->profile];

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
	layer += (rbox->top * img_columns + rbox->left) * layers;

	/* Number of grid columns */
	int columns = rbox->right - rbox->left + 1;

	/* Compute and execute the command for this parameters */
	struct command c = {
	  .x            = rbox->x & 31,
	  .v1           = v1,
	  .v2           = v2 ? v2 : v1,
	  .offset       = (rbox->y << 2) + (rbox->x >> 5),
	  .columns      = columns,
	  .masks        = masks + 2 * left_origin,
	  .real_start   = (left_origin > 0),
	  .vram_stride  = 4 - columns,
	  .data_stride  = ((img_columns - columns) << 2) * layers,
	  .gray         = (v2 != NULL),
	  .f            = f,
	};
	bopti_grid((void **)&layer, rbox->height, &c);
}

/* Specialized, faster version for single-column single-position instances */
void bopti_render_scsp(bopti_image_t const *img, struct rbox *rbox,
	uint32_t *v1, uint32_t *v2)
{
	/* Rendering function */
	bopti_asm_t f;
	if(v2) f.asm_gray_scsp = asm_gray_scsp[img->profile];
	else   f.asm_mono_scsp = asm_mono_scsp[img->profile];

	/* Compute the only rendering mask. Avoid UB if width = 32 */
	uint32_t mask = 0xffffffff;
	if(rbox->width < 32)
	{
		int right = 32 - ((rbox->visual_x & 31) + rbox->width);
		mask = ((1 << rbox->width) - 1) << right;
	}

	/* Number of layers */
	int layer_count[] = { 1, 2, 2, 3 };
	int layers = layer_count[img->profile];

	/* Number of columns in [img] */
	int img_columns = (img->width + 31) >> 5;

	/* Interwoven layer data. Skip left columns that are not rendered */
	const uint32_t *layer = (void *)img->data;
	layer += (rbox->top * img_columns + rbox->left) * layers;

	/* Starting value of VRAM pointers */
	int offset = (rbox->y << 2) + (rbox->visual_x >> 5);
	v1 += offset;
	if(v2) v2 += offset;

	/* Number of rows */
	int rows = rbox->height;
	/* Mask shift */
	int shift = -(rbox->x & 31);
	if(rbox->x < 0) shift += 32;

	/* Render the grid immediately; mono version */
	if(!v2) while(rows--)
	{
		f.asm_mono_scsp(v1, layer, mask, shift);
		layer += img_columns * layers;
		v1 += 4;
	}
	/* Gray version */
	else while(rows--)
	{
		f.asm_gray_scsp(v1, layer, mask, v2, shift);
		layer += img_columns * layers;
		v1 += 4;
		v2 += 4;
	}
}

void bopti_render_clip(int x, int y, bopti_image_t const *img, int left,
	int top, int width, int height, uint32_t *v1, uint32_t *v2)
{
	/* Adjust the bounding box of the input image */
	if(left < 0) width  += left, x -= left, left = 0;
	if(top  < 0) height += top,  y -= top,  top  = 0;
	if(left + width  > img->width)  width  = img->width  - left;
	if(top  + height > img->height) height = img->height - top;

	/* Intersect with the bounding box on-screen */
	if(x < 0) width  += x, left -= x, x = 0;
	if(y < 0) height += y, top  -= y, y = 0;
	if(x + width  > DWIDTH)  width  = DWIDTH  - x;
	if(y + height > DHEIGHT) height = DHEIGHT - y;

	/* Early finish for empty intersections */
	if(width <= 0 || height <= 0) return;

	/* Finish with the noclip variant */
	bopti_render_noclip(x, y, img, left, top, width, height, v1, v2);
}

void bopti_render_noclip(int visual_x, int y, bopti_image_t const *img,
	int left, int top, int width, int height, uint32_t *v1, uint32_t *v2)
{
	/* Start column and end column (both included) */
	int cl = (left) >> 5;
	int cr = (left + width - 1) >> 5;

	/* Finish with the standard bopti renderer */
	struct rbox rbox = { 0, visual_x, y, width, cl, cr, top, height };

	if(cl == cr && (visual_x & 31) + width <= 32)
	{
		rbox.x = (visual_x & 31) - (left & 31);
		bopti_render_scsp(img, &rbox, v1, v2);
	}
	else
	{
		/* x-coordinate of the first pixel of the first column */
		rbox.x = visual_x - (left & 31);
		bopti_render(img, &rbox, v1, v2);
	}
}
