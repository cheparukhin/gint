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

#include <gint/defs/types.h>

/* Expose the VRAM variable if GINT_NEED_VRAM is defined. It must always point
   to a 4-aligned buffer of size 1024. Any function can use it freely to:
   - Use another video ram area (triple buffering or more, gray engine);
   - Implement additional drawing functions;
   - Store data when not drawing. */
#ifdef GINT_NEED_VRAM
extern uint32_t *vram;
#endif

/* color_t - colors available for drawing
   The following colors are defined by the library:

     OPAQUE COLORS (override existing pixels)
       white, black	- the usual thing
       light, dark	- intermediate colors used with the gray engine

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

/* dupdate() - push the video RAM to the display driver
   This function makes the contents of the VRAM visible on the screen. It is
   the direct equivalent of Bdisp_PutDisp_DD(). */
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

   @x1 @y1 @x2 @y2  Bounding rectangle (drawn area).
   @color           Allowed colors: white, black, none, reverse */
void drect(int x1, int y1, int x2, int y2, color_t color);

//---
//	Point drawing functions
//---

/* dpixel() - change a pixel's color
   If the requested color is an operator, the result will depend on the current
   color of the pixel. Setting single pixels is the slowest method to produce a
   graphical result: all other functions for rendering lines, rectangles,
   images or text use highly-optimized methods, so check them out first if you
   care about performance.

   @x @y   Coordinates of the pixel to repaint
   @color  Allowed colors: white, black, none, reverse */
void dpixel(int x, int y, color_t color);

/* dline() - render a straight line
   This function draws a line using a Bresenham-style algorithm. Please note
   that the affected pixels may not be exactly the same when using dline() and
   Bdisp_DrawLineVRAM().

   dline() has optimization facilities for horizontal and vertical lines, but
   it does not detect if your line doesn't fit in the screen. So drawing from
   (-1e6,0) to (1e6,63) will work, but will be veeery slow.

   @x1 @y1 @x2 @y2  End points of the line (both included).
   @color           Allowed colors: white, black, none, reverse */
void dline(int x1, int y1, int x2, int y2, color_t color);

//---
//	Image rendering (bopti)
//---

/* image_t - image files encoded for bopti
   This format is the result of encoding images for bopti with the fxSDK's
   [fxconv] tool. The bopti routines can render it extremely fast, which makes
   it preferable over plain bitmaps if the images are never edited. */
typedef struct
{
	/* Image can only be rendered with the gray engine */
	uint gray	:1;
	/* Left for future use */
	uint		:3;
	/* Image profile (uniquely identifies a rendering function) */
	uint profile	:4;
	/* Full width, in pixels */
	uint width	:12;
	/* Full height, in pixels */
	uint height	:12;

	/* Raw layer data */
	uint8_t data[];

} GPACKED(4) image_t;

//---
//	Text rendering (topti)
//---

/* font_t - font data encoded for topti */
typedef struct
{
	/* Length of font name (not NUL-terminated) */
	uint title	:5;
	/* Font shape flags */
	uint bold	:1;
	uint italic	:1;
	uint serif	:1;
	uint mono	:1;
	/* Whether data is variable-length (proportional font) */
	uint prop	:1;
	/* Reserved for future use */
	uint		:2;
	/* Implemented charcter set */
	uint charset	:4;
	/* Line height */
	uint8_t line_height;
	/* Storage height */
	uint8_t data_height;

	/* The rest of the data depends on whether the font is proportional */
	union {
		/* For monospaced fonts */
		struct {
			/* Width of glyphs */
			uint16_t width;
			/* Storage size, in longwords, of each glyph */
			uint16_t storage_size;
			/* Raw glyph data */
			uint32_t data[];
		};
		/* For proportional fonts */
		struct {
			/* Storage index to find glyphs quickly */
			uint16_t index[16];
			/* Size array (padded to 4 bytes), 1 byte per entry,
			   followed by glyph data */
			uint8_t sized_data[];
		};
	};

	/* The font name is stored after the data. The size is the length set
	   in the [title] field, padded to 4 bytes with NULs. There might not
	   be a NUL at the end. */

} GPACKED(4) font_t;

/* dfont() - set the default font for text rendering
   This font will be used by dtext() and sister functions. If font=NULL, gint's
   default 5*6 font is used.

   @font  Font to use for subsequent text rendering calls */
void dfont(font_t const * font);

/* dsize() - get the width and height of rendered text
   This function computes the size that the given string would take up if
   rendered with a certain font. If you specify a NULL font, the currently
   configured font will be used; this differs from  dfont() which uses gint's
   default font when NULL is passed.

   Note that the height of each glyph is not stored in the font, only the
   maximum. Usually this is what you want because vertically-centered strings
   must have the same baseline regardless of their contents. So the height
   returned by dsize() is the same for all strings, only depends on the font.

   The height is computed in constant time, and the width in linear time.

   @str   String whose size must be evaluated
   @font  Font to use; if NULL, defaults to the current font
   @w @h  Set to the width and height of the rendered text, may be NULL */
void dsize(const char *str, font_t const * font, int *w, int *h);

/* dtext() - display a string of text

   Draws some text in the video RAM using the font set with dfont() (or gint's
   default if no such font was set).

   Due to the particular design of topti, this function takes advantage of the
   line structure of the VRAM to rendeer several characters at once. This is
   not a printf()-family function so [str] cannot contain formats like "%d"
   (they will be rendered directly) and cannot receive additional arguments.

   @x @y   Coordinates of top-left corner of the rendered string
   @str    String to display
   @fg     Text color
   @bg     Background color, pass [color_none] to disable */
void dtext(int x, int y, const char *str, color_t fg, color_t bg);

#endif /* FX9860G */

#endif /* GINT_DISPLAY_FX */
