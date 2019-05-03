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

/* dupdate() - push the video RAM to the display driver
   This function makes the contents of the VRAM visible on the screen. It is
   the direct equivalent of Bdisp_PutDisp_DD().

   If triple buffering is enabled (this is the default, and disabled only if
   dvram() is used to setup double buffering instead), it also swaps buffers.
   Also waits for the previous dupdate() call to finish before executing. */
void dupdate(void);

//---
//	Area rendering functions
//---

/* dclear() - fill the screen with a single color
   This function clears the screen by painting all the pixels in a single,
   opaque color.

   @color  Any R5G6B5 color */
void dclear(uint16_t color);

/* drect() - fill a rectangle of the screen
   This functions paints a rectangle in an opaque color. The endpoints (x1 y1)
   and (x2 y2) are included in the rectangle.

   @x1 @y1 @x2 @y2  Bounding rectangle (drawn area).
   @color           Any R5G6B5 color */
void drect(int x1, int y1, int x2, int y2, uint16_t color);

//---
//	Point drawing functions
//---

/* dpixel() - change a pixel's color
   Paints the selected pixel with an opaque color. Setting pixels individually
   is a slow method for rendering. Other functions that draw lines, rectangles,
   images or text will take advantage of possible optimizations to make the
   rendering faster: check them out first.

   @x @y   Coordinates of the pixel to repaint
   @color  Any R5G6B5 color */
void dpixel(int x, int y, uint16_t color);

/* dline() - render a straight line
   This function draws a line using a Bresenham-style algorithm. Please note
   that the affected pixels may not be exactly the same when using dline() and
   Bdisp algorithms.

   dline() has optimization facilities for horizontal and vertical lines. The
   first kind is about twice as fast, while the second avoids some computation
   (the optimization gain is not as significant as on fx9860g). dline() is not
   able to clip the line without calculating all the pixels, so drawing a line
   from (-1e6,0) to (1e6,395) will work, but will be veeery slow.

   @x1 @y1 @x2 @y2  End points of the line (both included).
   @color           Any R5G6B5 color */
void dline(int x1, int y1, int x2, int y2, uint16_t color);

//---
//	Image rendering (bopti)
//---

//---
//	Text rendering (topti)
//---

typedef void font_t;

#endif /* FXCG50 */

#endif /* GINT_DISPLAY_CG */
