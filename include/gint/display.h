//---
//	gint:display - Drawing functions
//
//	This module covers the drawing functions that are common to fx9860g and
//	fxcg50. Platform-specific definitions are found in <gint/display-fx.h>
//	and <gint/display-cg.h>.
//---

#ifndef GINT_DISPLAY
#define GINT_DISPLAY

#include <gint/defs/types.h>

/* Platform-specific functions include VRAM management and the definition of
   the color_t type. */

#ifdef FX9860G
#include <gint/display-fx.h>
#endif

#ifdef FXCG50
#include <gint/display-cg.h>
#endif

/* TODO: dinfo() or similar */

//---
//	Video RAM management
//---

/* dupdate(): Push the video RAM to the display driver
   This function makes the contents of the VRAM visible on the screen. It is
   the direct equivalent of Bdisp_PutDisp_DD().

   On fxcg50, if triple buffering is enabled (which is the default and disabled
   only by calling dvram()), it also swaps buffers. Due to the DMA being used,
   always waits for the previous call to dupdate() call() to finish. */
void dupdate(void);

//---
//	Area rendering functions
//---

/* dclear(): Fill the screen with a single color
   This function clears the screen by painting all the pixels in a single
   color. It is optimized for opaque colors.

   On fx9860g, use drect() if you need complex operators such as invert.

   @color  fx9860g: white, black
           fxcg50:  Any R5G6B5 color */
void dclear(color_t color);

/* drect(): Fill a rectangle of the screen
   This functions applies a color or an operator to a rectangle defined by two
   points (x1 y1) and (x2 y2). Both are included in the rectangle.

   @x1 @y1 @x2 @y2  Bounding rectangle (drawn area).
   @color           fx9860g: white, black, none, invert
                    fxcg50:  Any R5G6B5 color */
void drect(int x1, int y1, int x2, int y2, color_t color);

/* drect_border(): Rectangle with border
   This function draws a rectangle with an inner border. The border width must
   be smaller than half the width and half the height. */
void drect_border(int x1, int y1, int x2, int y2, int fill_color,
	int border_width, int border_color);

//---
//	Point drawing functions
//---

/* dpixel(): Change a pixel's color
   Paints the selected pixel with an opaque color. Setting pixels individually
   is a slow method for rendering. Other functions that draw lines, rectangles,
   images or text will take advantage of possible optimizations to make the
   rendering faster: check them out first.

   On fx9860g, if an operator such as invert is used, the result will depend
   on the current color of the pixel.

   @x @y   Coordinates of the pixel to repaint
   @color  fx9860g: white, black, none, invert
           fxcg50:  Any R5G6B5 color */
void dpixel(int x, int y, color_t color);

/* dline(): Render a straight line
   This function draws a line using a Bresenham-style algorithm. Please note
   that dline() may not render lines exactly like Bdisp_DrawLineVRAM().

   dline() has optimization facilities for horizontal and vertical lines. The
   speedup for horizontal lines is about x2 on fxcg50 and probably about x30
   on fx9860g. Vertical lines have a smaller speedup.

   dline() is currently not able to clip arbitrary lines without calculating
   all the pixels, so drawing a line from (-1e6,0) to (1e6,395) will work but
   will be veeery slow.

   @x1 @y1 @x2 @y2  End points of the line (both included).
   @color           fx9860g: white, black, none, invert
                    fxcg50:  Any R5G6B5 color */
void dline(int x1, int y1, int x2, int y2, color_t color);

/* dhline(): Full-width horizontal line
   This function draws a horizontal line from the left end of the screen to the
   right end, much like the Basic command "Horizontal".

   @y      Line number
   @color  fx9860g: white, black, none, invert
           fxcg50:  Any R5G6B5 color */
void dhline(int y, color_t color);

/* dvline(): Full-height vertical line
   This function draws a vertical line from the top end of the screen to the
   bottom end, much like the Basic command "Vertical".

   @x      Column number
   @color  fx9860g: white, black, none, invert
           fxcg50:  Any R5G6B5 color */
void dvline(int x, color_t color);

//---
//	Text rendering (topti)
//---

/* font_t - font data encoded for topti */
typedef struct
{
	/* Length of font name (not necessarily NUL-terminated!) */
	uint title    :5;
	/* Font shape flags */
	uint bold     :1;
	uint italic   :1;
	uint serif    :1;
	uint mono     :1;
	/* Whether data is variable-length (proportional font) */
	uint prop     :1;
	/* Reserved for future use */
	uint          :2;
	/* Implemented charcter set */
	uint charset  :4;
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

/* dfont(): Set the default font for text rendering
   This font will be used by dtext() and sister functions. If [font = NULL],
   gint's default font is used.

   On fx9860g, the default font is a 5x7 font very close to the system's.
   On fxcg50, the default font is an original 8x9 font.

   @font  Font to use for subsequent text rendering calls
   Returns the previously configured font. */
font_t const *dfont(font_t const * font);

/* dsize(): Get the width and height of rendered text
   This function computes the size that the given string would take up if
   rendered with a certain font. If you specify a NULL font, the currently
   configured font will be used; this is different from dfont(), which uses
   gint's default font when NULL is passed.

   Note that the height of each glyph is not stored in the font, only the
   maximum. Usually this is what you want because vertically-centered strings
   must have the same baseline regardless of their contents. So the height
   returned by dsize() is the same for all strings, only depends on the font.

   The height is computed in constant time, and the width in linear time. If
   [w = NULL], this function returns in constant time.

   @str   String whose size must be evaluated
   @font  Font to use; if NULL, defaults to the current font
   @w @h  Set to the width and height of the rendered text, may be NULL */
void dsize(char const *str, font_t const * font, int *w, int *h);

/* dtext(): Display a string of text

   Draws some text in the video RAM using the font set with dfont() (or gint's
   default if no such font was set).

   On fx9860g, due to the particular design of topti, this function performs
   drastic rendering optimizations using the line structure of the VRAM and is
   able to render several characters at once.

   This is not a printf()-family function so [str] cannot contain formats like
   "%d" and you cannot pass additional arguments.

   @x @y   Coordinates of top-left corner of the rendered string
   @str    String to display
   @fg     Text color
           fx9860g: white, black, none, invert
           fxcg50:  Any R5G6B6 color, or C_NONE
   @bg     Background color
           fx9860g: white, black, none, invert
           fxcg50:  Any R5G6B5 color, or C_NONE */
void dtext(int x, int y, char const *str, int fg, int bg);

/* dprint(): Display a formatted string
   Much like dtext(), but accepts printf-like formats with arguments. See
   <gint/std/stdio.h> for a detailed view of what this format supports. */
void dprint(int x, int y, int fg, int bg, char const *format, ...);

//---
//	Image rendering (bopti)
//---

/* The bopti_image_t structure is platform-dependent. */

/* dimage(): Render a full image
   This function blits an image on the VRAM using gint's special format. It is
   a special case of dsubimage() where the full image is drawn with clipping.

   @x @y   Coordinates of the top-left corner of the image
   @image  Pointer to image encoded with [fxconv] */
void dimage(int x, int y, bopti_image_t const *image);

/* Option values for dsubimage() */
enum {
	/* No option */
	DIMAGE_NONE = 0x00,

	/* Disable clipping, ie. adjustments to the specified subrectangle and
	   screen location such that any part that overflows from the image or
	   the screen is ignored. Slightly faster. */
	DIMAGE_NOCLIP = 0x01,
};

/* dsubimage(): Render a section of an image
   This function blits a subrectangle [left, top, width, height] of an image on
   the VRAM. It is more general than dimage() and also provides a few options.

   @x @y           Coordinates on screen of the rendered subrectangle
   @image          Pointer to image encoded with [fxconv]
   @left @top      Top-left coordinates of the subrectangle within [image]
   @width @height  Subrectangle dimensions
   @flags          OR-combination of DIMAGE_* flags */
void dsubimage(int x, int y, bopti_image_t const *image, int left, int top,
	int width, int height, int flags);

//---
//	Advanced functions
//---

/* dupdate_noint(): Push VRAM to the display without interrupts
   This function does exactly as dupdate(), but does not use interrupts and
   always returns when the transfer is finished. It actively waits for the end
   of the transfer and is thus bad for any general-purpose use. In fact, it is
   only needed to display panic messages in exception handlers. Don't use it
   unless you know precisely why you're doing it. */
void dupdate_noint(void);

#endif /* GINT_DISPLAY */
