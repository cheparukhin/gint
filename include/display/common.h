//---
//	display:common - Internal definitions for common display functions
//---

#ifndef DISPLAY_COMMON
#define DISPLAY_COMMON

#include <gint/display.h>

/* gint_dhline(): Optimized horizontal line
   @x1 @x2 @y  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dhline(int x1, int x2, int y, color_t color);

/* gint_dvline(): Optimized vertical line
   @y1 @y2 @x  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dvline(int y1, int y2, int x, color_t color);

//---
//	Font rendering (topti)
//---

/* Current font */
extern font_t const * topti_font;
/* Default font */
extern font_t const * gint_default_font;

/* enum topti_charset: Available character set decoders
   Each charset is associated with a reduced character table. */
enum topti_charset
{
	charset_numeric  = 0,	/*  10 elements: 0..9 */
	charset_upper    = 1,	/*  26 elements: A..Z */
	charset_alpha    = 2,	/*  52 elements: A..Z, a..z */
	charset_alnum    = 3,	/*  62 elements: A..Z, a..z, 0..9 */
	charset_print    = 4,	/*  95 elements: 0x20..0x7e  */
	charset_ascii    = 5,	/* 128 elements: 0x00..0x7f */
};

/* charset_size(): Number of elements in each character set
   @set  Character set ID
   Returns the expected number of glyphs, -1 if charset ID is invalid. */
int charset_size(enum topti_charset set);

/* charset_decode(): Translate ASCII into reduced character sets
   Returns the position of [c] in the character table of the given charset, or
   -1 if [c] is not part of that set.
   @set  Any character set
   @c    Character to decode */
int charset_decode(enum topti_charset set, uint c);

/* topti_offset(): Use a font index to find the location of a glyph
   @f      Font object
   @glyph  Glyph number obtained by charset_decode(), must be nonnegative.
   Returns the offset the this glyph's data in the font's data array. When
   using a proportional font, the size array is not heeded for. */
int topti_offset(font_t const *f, uint glyph);

#endif /* DISPLAY_COMMON */
