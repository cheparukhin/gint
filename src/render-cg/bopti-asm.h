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

#endif /* GINT_RENDERCG_BOPTIASM */
