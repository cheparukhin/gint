//---
//	gint:display-fx - fx9860g drawing functions
//
//	This module is in charge of all monochrome rendering. The gray engine
//	has its own functions, but often relies on this module (because the
//	gray effect is created through two monochrome buffers).
//---

#ifndef GINT_DISPLAY_FX
#define GINT_DISPLAY_FX

#ifdef FX9860G

/* Screen dimensions on fx9860g */
#define DWIDTH 128
#define DHEIGHT 64

/* color_t - colors available for drawing
   The following colors are defined by the library:

     OPAQUE COLORS (override existing pixels)
       white, black	- the usual thing
       light, dark	- additional colors used by the gray engine

     OPERATORS (combine with existing pixels)
       none		- leaves unchanged
       reverse		- inverts white <-> black, light <-> dark
       lighten		- shifts black -> dark -> light -> white -> white
       darken		- shifts white -> light -> dark -> black -> black

   Not all colors can be used with all functions. To avoid ambiguities, all
   functions explicitly indicate compatible colors. */
typedef enum
{
	/* Opaque colors */
	color_white	= 0,
	color_light	= 1,
	color_dark	= 2,
	color_black	= 3,

	/* Monochrome operators */
	color_none	= 4,
	color_reverse	= 5,

	/* Gray operators */
	color_lighten	= 6,
	color_darken	= 7,

} color_t;

//---
//	Area drawing functions
//---

/* dupdate() - pushes the video RAM to the display driver
   This function makes the contents of the VRAM visible on the screen. It is
   the direct equivalent of Bdisp_PutDisp_DD() */
void dupdate(void);

/* dclear() - fill the screen with a single color
   This function clears the screen by replacing all the pixels with a single
   color. This function is optimized for opaque drawing. If you wish to apply
   operators, use drect().

   @color  Allowed colors: white, black */
void dclear(color_t color);

/* drect() - fill a rectangle of the screen
   This functions applies a color or an operator to a rectangle defined by two
   points (x1 y1) and (x2 y2). Both are included in the rectangle.

   If GINT_LAX is defined, this function makes the following assumptions:
     0 <= x1 < x2 <= 127
     0 <= y1 < y2 <= 63

   @x1 @y1 @x2 @y2  Bounding rectangle (drawn area).
   @color           Allowed colors: white, black, none, reverse */
void drect(int x1, int y1, int x2, int y2, color_t color);

//---
//	Point drawing functions
//---

/* dpixel() - change a pixel's color
   If the requested color is an operator, the result will depend on the current
   color of the pixel.

   If GINT_LAX is defined, this function makes the following assumptions:
     0 <= x <= 127
     0 <= y <= 63

   @x @y   Coordinates of the pixel to repaint
   @color  Allowed colors: white, black, none, reverse */
void dpixel(int x, int y, color_t color);

/* dline() - render a straight line
   This function draws a line using a Bresenham-style algorithm. Please note
   that the affected pixels may not be exactly the same when using dline() and
   Bdisp_DrawLineVRAM().

   dline() has optimization facilities for horizontal and vertical lines.

   If GINT_LAX is defined, this function makes the following assumptions:
     0 <= x1 <= x2 <= 127
     0 <= y1, y2 <= 63

   @x1 @y1 @x2 @y2  End points of theline (both included).
   @color           Allowed colors: white, black, none, reverse */
void dline(int x1, int y1, int x2, int y2, color_t color);

#endif /* FX9860G */

#endif /* GINT_DISPLAY_FX */
