//---
//	gint:display-cg - fxcg50 rendering functions
//
//	This module covers all 16-bit opaque rendering functions. For
//	gamma-related functions, color composition, check out a color library.
//
//	All the functions in this module work on a 396x224 resolution - gint
//	lets you use the full surface!
//---

#ifndef GINT_DISPLAY_CG
#define GINT_DISPLAY_CG

#ifdef FXCG50

#include <gint/defs/types.h>

/* Expose the VRAM variable if GINT_NEED_VRAM is defined. It must always point
   to a 32-aligned buffer of size 177408. Any function can use it freely to
   perform rendering or store data when not drawing. Triple buffering is
   already implemened in gint, see the dvram() function below.

   In this module, colors are in the 16-bit R5G6B5 format, as it is the format
   used by the display controller. */
#ifdef GINT_NEED_VRAM
extern uint16_t *vram;
#endif

/* Provide a platform-agnostic definition of color_t.
   Some functions also support transparency, in which case they take an [int]
   as argument and recognize negative values as transparent. */
typedef uint16_t color_t;

enum {
	/* Compatibility with fx9860g color names */
	color_white = 0xffff,
	color_light = 0xaaaa,
	color_dark  = 0x5555,
	color_black = 0x0000,

	color_none = -1,
};

//---
//	Video RAM management
//---

/* dvram() - Control video RAM address and triple buffering

   Normal rendering under gint uses double-buffering: there is one image
   displayed on the screen and one in memory, in a region called the video RAM
   (VRAM). The application draws frames in the VRAM then sends them to the
   screen only when they are finished, using dupdate().

   On fxcg50, the performance bottleneck is almost always the graphical
   rendering (especially in games) because the high amount of data, 173 kB per
   frame in full-resolution, makes graphics manipulation computationally
   expensive. The transfer also takes about 10 ms in itself.

   Since gint transfers data to the screen using the DMA, it is possible to run
   the application while the finished frame is being transferred. However,
   writing to the VRAM during this period it is still begin read by the DMA.
   Changing the contents of the VRAM too soon would alter the frame being sent.

   The solution to this is to use triple-buffering with the display and two
   VRAMs that are alternately begin written to while the other is being
   transferred. The VRAM switching is handled by dupdate() and is activated
   whenever two VRAMs are configured.

   By default gint uses triple buffering with one VRAM in the user stack and
   a second one in the system stack.

   VRAMs must be contiguous, 32-aligned, (2*396*224)-byte buffers.

   @main       Main VRAM area, used alone if [secondary] is NULL
   @secondary  Additional VRAM area, enables triple buffering if non-NULL */
void dvram(uint16_t *main, uint16_t *secondary);

#endif /* FXCG50 */

#endif /* GINT_DISPLAY_CG */
