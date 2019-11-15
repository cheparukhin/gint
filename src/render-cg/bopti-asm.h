//---
//	gint:render-cg:bopti-asm - Assembler drawing routines for bopti
//---

#ifndef GINT_RENDERCG_BOPTIASM
#define GINT_RENDERCG_BOPTIASM

/* bopti_r5g6b5(): Full opaque 16-bit rendering
   @data        Input data (2-aligned)
   @target      Target in VRAM (2-aligned)
   @width       Width of rendered image
   @height      Height of rendered image
   @in_stride   Bytes to ignore between two rows of @data
   @out_stride  Bytes to ignore between two rows of @target */
void bopti_r5g6b5(uint16_t const *data, uint16_t *target, int width,
	int height, int in_stride, int out_stride);

/* bopti_r5g6b5a(): 16-bit rendering with alpha
   @data @target @width @height @in_stride @out_stride
           As in bopti_r5g6b5().
   @alpha  Color code that encodes transparency */
void bopti_r5g6b5a(uint16_t const *data, uint16_t *target, int width,
	int height, int in_stride, int out_stride, uint16_t alpha);

/* bopti_p8(): 8-bit palette rendering with alpha
   @data @target @width @height @in_stride @out_stride
            As in bopti_r5g6b5().
   @palette Pointer to image palette (256 colors)
   @alpha   Palette index for transparency, if >= 256 no alpha is used */
void bopti_p8(uint8_t const *data, uint16_t *target, int width, int height,
	int in_stride, int out_stride, uint16_t const *palette, int alpha);

/* bopti_p4(): 4-bit palette rendering with alpha
   @data @target @width @height @in_stride @out_stride
            As in bopti_r5g6b5().
   @palette Pointer to image palette (16 colors)
   @alpha   Palette index for transparency, if >= 16 no alpha is used
   @offset  Initial offset within [data], in number of pixels */
void bopti_p4(uint8_t const *data, uint16_t *target, int width, int height,
	int in_stride, int out_stride, uint16_t const *palette, int alpha,
	int offset);

/* bopti_render_clip(): Render with clipping
   Same parameters as dsubimage(), except for flags. */
void bopti_render_clip(int x, int y, image_t const *img, int left, int top,
	int width, int height);

/* bopti_render_clip(): Render without clipping
   Behaviour is not defined if (left,top)+(width,height) is not included in the
   source image or (x,y)+(width,height) is not included in the VRAM area.
   Same parameters as dsubimage(), except for flags. */
void bopti_render_noclip(int x, int y, image_t const *img, int left, int top,
	int width, int height);

#endif /* GINT_RENDERCG_BOPTIASM */
