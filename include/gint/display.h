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
   the equivalent of Bdisp_PutDisp_DD() in most situations.

   On fx-9860G, this function also manages the gray engine settings. When the
   gray engine is stopped, it pushes the contents of the VRAM to screen, and
   when it is on, it swaps buffer and lets the engine's timer push the VRAMs to
   screen when suitable. To make the transition between the two modes smooth,
   dgray() does not enable the gray engine immediately; instead the first call
   to update() after dgray() switches the gray engine on and off.

   On fx-CG 50, because rendering is slow and sending data to screen is also
   slow, a special mechanism known as triple buffering is implemented. Two
   VRAMs are allocated, and frames are alternately rendered on each VRAM. When
   dupdate() is called to push one of the VRAMs to screen, it starts the DMA
   (which performs the push in the background) and immediately returns after
   switching to the other VRAM so that the user can draw during the push.

   The transfer from the DMA to the screen lasts about 10 ms if memory is
   available every cycle. Each access to the memory delays the transfer by a
   bit, so to fully use the available time, try to run AIs, physics or other
   calculation-intensive code rather than using the VRAM.

   Typical running time without overclock:
   fx-9860G: 1 ms (gray engine off)
   fx-9860G: ~30% of CPU time (gray engine on)
   fx-CG 50: 11 ms */
void dupdate(void);

//---
//	Area rendering functions
//---

/* dclear(): Fill the screen with a single color

   This function clears the screen by painting all the pixels in a single
   color. It is optimized for opaque colors; on fx-CG 50, it uses dma_memset()
   to fill in the VRAM.

   Typical running time without overclock:
   fx-9860G SH3:  70 µs
   fx-9860G SH4:  15 µs
   fx-CG 50:      2.5 ms (full-screen drect() is about 6 ms)

   On fx9860g, use drect() if you need operators that modify existing pixels
   instead of replacing them such as invert.

   @color  fx-9860G: white light* dark* black
           fx-CG 50: Any R5G6B5 color

   *: When the gray engine is on, see dgray(). */
void dclear(color_t color);

/* drect(): Fill a rectangle of the screen
   This functions applies a color or an operator to a rectangle defined by two
   points (x1 y1) and (x2 y2). Both are included in the rectangle.

   @x1 @y1  Top-left rectangle corner (included)
   @x2 @y2  Bottom-right rectangle corner (included)
   @color  fx-9860G: white light* dark* black none invert lighten* darken*
           fx-CG 50: Any R5G6B5 color or C_NONE

   *: When the gray engine is on, see dgray(). */
void drect(int x1, int y1, int x2, int y2, int color);

/* drect_border(): Rectangle with border
   This function draws a rectangle with an inner border. The border width must
   be smaller than half the width and half the height.

   @x1 @y1 @x2 @y2  Top-left and bottom-right rectangle corners (included)
   @fill_color      Center color (same values as drect() are allowed)
   @border_width    Amount of pixels reserved for the border on each side
   @border_color    Border color (same values as drect() are allowed) */
void drect_border(int x1, int y1, int x2, int y2,
	int fill_color, int border_width, int border_color);

//---
//	Point drawing functions
//---

/* dpixel(): Change a pixel's color

   Paints the selected pixel with an opaque color or applies an operator to a
   pixel. Setting pixels individually is a slow method for rendering. Other
   functions that draw lines, rectangles, images or text will take advantage of
   possible optimizations to make the rendering faster, so prefer using them
   when they apply.

   On fx-9860G, if an operator such as invert is used, the result will depend
   on the current color of the pixel.

   @x @y   Coordinates of the pixel to repaint
   @color  fx-9860G: white light* dark* black none invert lighten* darken*
           fx-CG 50: Any R5G6B5 color or C_NONE

   *: When the gray engine is on, see dgray(). */
void dpixel(int x, int y, int color);

/* dline(): Render a straight line

   This function draws a line using a Bresenham-style algorithm. Please note
   that dline() may not render lines exactly like Bdisp_DrawLineVRAM().

   dline() has optimizations for horizontal and vertical lines. The speedup for
   horizontal lines is about x2 on fx-CG 50 and probably about x30 on fx-9860G.
   Vertical lines have a smaller speedup.

   dline() is currently not able to clip arbitrary lines without calculating
   all the pixels, so drawing a line from (-1e6,0) to (1e6,395) will work but
   will be very slow.

   @x1 @y1 @x2 @y2  Endpoints of the line (both included).
   @color           Line color (same values as dpixel() are allowed) */
void dline(int x1, int y1, int x2, int y2, int color);

/* dhline(): Full-width horizontal line
   This function draws a horizontal line from the left end of the screen to the
   right end, much like the Basic command "Horizontal".

   @y      Line number
   @color  Line color (same values as dline() are allowed) */
void dhline(int y, int color);

/* dvline(): Full-height vertical line
   This function draws a vertical line from the top end of the screen to the
   bottom end, much like the Basic command "Vertical".

   @x      Column number
   @color  Line color (same values as dline() are allowed) */
void dvline(int x, int color);

//---
//	Text rendering (topti)
//---

/* font_t: Font data encoded for topti */
typedef struct
{
	/* Font name (NUL-terminated), NULL if no title */
	char const *name;

	/* Font shape flags */
	uint bold    :1;
	uint italic  :1;
	uint serif   :1;
	uint mono    :1;
	uint         :3;
	/* Whether data is variable-length (proportional font) */
	uint prop    :1;

	/* Line height */
	uint8_t line_height;
	/* Storage height */
	uint8_t data_height;

	/* Number of Unicode blocks */
	uint8_t block_count;
	/* Number of total glyphs */
	uint32_t glyph_count;

    /* Character spacing (usually 1) */
    uint8_t char_spacing;

    uint :24;

	struct {
		/* Unicode point of first character in block */
		uint start   :20;
		/* Length of block */
		uint length  :12;
	} *blocks;

	/* Raw glyph data */
	uint32_t *data;

	union {
		/* For monospaced fonts */
		struct {
			/* Width of glyphs */
			uint16_t width;
			/* Storage size, in longwords, of each glyph */
			uint16_t storage_size;
		};
		/* For proportional fonts */
		struct {
			/* Storage index to find glyphs quickly */
			uint16_t *glyph_index;
			/* Width of each individual glyph */
			uint8_t *glyph_width;
		};
	};

} GPACKED(4) font_t;

/* dfont(): Set the default font for text rendering

   This function changes the default font for text rendering; this affects
   dtext_opt(), dtext() and the related functions. If the specified font is
   NULL, gint's default font is used instead.

   On fx-9860G, the default font is a 5x7 font very close to the system's.
   On fx-CG 50, the default font is an original, proportional 8x9 font.

   This function returns the previously configured font. Normally you want to
   restore it after you're done so as to not affect ambiant calls to text
   rendering functions. It would look like this:

     font_t const *old_font = dfont(new_font);
     // Do the text rendering...
     dfont(old_font);

   @font  Font to use for subsequent text rendering calls
   Returns the previously configured font. */
font_t const *dfont(font_t const *font);

/* dsize(): Get the width and height of rendered text

   This function computes the size that the given string would take up if
   rendered with a certain font. If you specify a NULL font, the currently
   configured font will be used; this is different from dfont(), which uses
   gint's default font when NULL is passed.

   Note that the height of each glyph is not stored in the font, only the
   maximum. Usually this is what you want because vertically-centered strings
   must have the same baseline regardless of their contents. So the height
   set by dsize() is the same for all strings and only depends on the font.

   The height is computed in constant time, and the width in linear time. If
   the third argument is NULL, this function returns in constant time.

   @str   String whose size must be evaluated
   @font  Font to use; if NULL, defaults to the current font
   @w @h  Set to the width and height of the rendered text, may be NULL */
void dsize(char const *str, font_t const *font, int *w, int *h);

/* Alignment settings for dtext_opt() and dprint_opt(). Combining a vertical
   and a horizontal alignment option specifies where a given point (x,y) should
   be relative to the rendered string. */
enum {
	/* Horizontal settings: default in dtext() is DTEXT_LEFT */
	DTEXT_LEFT   = 0,
	DTEXT_CENTER = 1,
	DTEXT_RIGHT  = 2,
	/* Vertical settings: default in dtext() is DTEXT_TOP */
	DTEXT_TOP    = 0,
	DTEXT_MIDDLE = 1,
	DTEXT_BOTTOM = 2,
};

/* dtext_opt(): Display a string of text

   Draws some text in the video RAM using the font set with dfont() (or gint's
   default if no such font was set). This function has a lot of parameters,
   see dtext() for a simpler version.

   The alignment options specify where (x y) should be relative to the rendered
   string. The default is halign=DTEXT_LEFT and valign=DTEXT_TOP, which means
   that (x y) is the top-left corder of the rendered string. The different
   combinations of settings provide 9 positions of (x y) to choose from.

   On fx-9860G, due to the particular design of topti, this function performs
   drastic rendering optimizations using the line structure of the VRAM and
   renders 5 or 6 characters simultaneously.

   This is not a printf()-family function so str cannot contain formats like
   "%d" and you cannot pass additional arguments. See dprint_opt() and dprint()
   for that.

   @x @y    Coordinates of the anchor of the rendered string
   @fg @bg  Text color and background color
            fx-9860G: white light* dark* black none invert lighten* darken*
            fx-CG 50: Any R5G6B6 color, or C_NONE
   @halign  Where x should be relative to the rendered string
   @valign  Where y should be relative to the rendered string
   @str     String to display */
void dtext_opt(int x, int y, int fg, int bg, int halign, int valign,
	char const *str);

/* dtext(): Simple version of dtext_opt() with defaults
   Calls dtext_opt() with bg=C_NONE, halign=DTEXT_LEFT and valign=DTEXT_TOP. */
void dtext(int x, int y, int fg, char const *str);

/* dprint_opt(): Display a formatted string

   This function is exactly like dtext_opt(), but accepts printf-like formats
   with arguments. See <gint/std/stdio.h> for a detailed view of what this
   format supports. For example:

     dprint_opt(x, y, fg, bg, halign, valign, "A=%d B=%d", A, B); */
void dprint_opt(int x, int y, int fg, int bg, int halign, int valign,
	char const *format, ...);

/* dprint(): Simple version of dprint_op() with defaults
   Calls dprint_opt() with bg=C_NONE, halign=DTEXT_LEFT and valign=DTEXT_TOP */
void dprint(int x, int y, int fg, char const *format, ...);

//---
//	Image rendering (bopti)
//---

/* The bopti_image_t structure is platform-dependent, see <gint/display-fx.h>
   and <gint/display-cg.h> if you're curious. */

/* dimage(): Render a full image
   This function blits an image on the VRAM using gint's special format. It is
   a special case of dsubimage() where the full image is drawn with clipping.

   @x @y   Coordinates of the top-left corner of the image
   @image  Pointer to image encoded with fxconv for bopti */
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
   @image          Pointer to image encoded with fxconv for bopti
   @left @top      Top-left coordinates of the subrectangle within the image
   @width @height  Subrectangle dimensions
   @flags          OR-combination of DIMAGE_* flags */
void dsubimage(int x, int y, bopti_image_t const *image, int left, int top,
	int width, int height, int flags);

#endif /* GINT_DISPLAY */
