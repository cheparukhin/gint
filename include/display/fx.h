//---
//	display:fx - Internal definitions for the display module on fx9860g
//---

#ifndef DISPLAY_FX
#define DISPLAY_FX

#include <gint/defs/types.h>
#include <gint/display.h>

/* masks() - compute the vram masks for a given rectangle

   Since the VRAM is line-based with four uin32_t elements per row, we can
   execute any operation on a rectangle by running it on each set of four
   uint32_t elements.

   This function calculates four uint32_t values and stores them in @mask. Each
   of the 128 bits in this array represents a column of the screen, and the bit
   of column c is 1 iff x1 <= c <= x2.

   These masks can then be and-ed/or-ed/anything on the VRAM to draw.

   @x1 @x2  Targeted screen range, horizontally (both included)
   @masks   Stores the result of the function (four uint32_t values) */
void masks(int x1, int x2, uint32_t *masks);

/* Font currently configured for text rendering */
extern font_t const * topti_font;

#endif /* DISPLAY_FX */
