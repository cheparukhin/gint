#define GINT_NEED_VRAM
#include <gint/defs/types.h>
#include <gint/display.h>
#include "topti-asm.h"

/* Default font */
extern font_t gint_font5x6;
font_t const * topti_font = &gint_font5x6;

/* dfont() - set the default font for text rendering */
void dfont(font_t const * font)
{
	topti_font = font ? font : &gint_font5x6;
}

/* enum charset: Available character set decoders
   Each charset is associated with a reduced character table. */
enum charset
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
int charset_size(enum charset set)
{
	int size[] = { 10, 26, 52, 62, 95, 128 };
	return (uint)set < 6 ? size[set] : -1;
}

/* charset_decode(): Translate ASCII into reduced character sets
   Returns the position of [c] in the character table of the given charset, or
   -1 if [c] is not part of that set.
   @set  Any character set
   @c    Character to decode */
int charset_decode(enum charset set, uint c)
{
	int x, y;

	switch(set)
	{
	case charset_numeric:
		x = c - '0';
		return (x < 10) ? x : -1;
	case charset_upper:
		x = (c - 'A') & ~0x20;
		return (x < 26) ? x : -1;
	case charset_alnum:
		x = c - '0';
		if(x < 10) return x;
		/* Intentional fallthrough */
	case charset_alpha:
		y = c & 0x20;
		x = (c ^ y) - 'A';
		/* Turn 32 into 26 and leave 0 as 0 */
		y = y - (y >> 3) - (y >> 4);
		return (x < 26) ? (x + y) : -1;
	case charset_print:
		x = c - 0x20;
		return (x < 0x5f) ? x : -1;
	case charset_ascii:
		return c;
	}

	return -1;
}

/* topti_offset(): Use a font index to find the location of a glyph
   @f      Font object
   @glyph  Glyph number obtained by charset_decode(), must be nonnegative.
   Returns the offset the this glyph's data in the font's data array. When
   using a proportional font, the size array is not heeded for. */
int topti_offset(font_t const *f, uint glyph)
{
	/* Non-proportional fonts don't need an index */
	if(!f->prop) return glyph * f->storage_size;

	uint8_t const *width = f->sized_data;

	/* The index gives us the position of all glyphs whose IDs are mutiples
	   of 8. Start with a close one and iterate from there. */
	uint g = glyph & ~0x7;
	int offset = f->index[g >> 3];

	/* Traverse the width array (which is in bits) while converting to
	   longword size */
	while(g < glyph) offset += (width[g++] * f->data_height + 31) >> 5;

	return offset;
}

/* topti_split(): Split glyph data into lines
   This function splits the data from [glyph] inyo lines and writes a bit of
   each line in [operators]. This operation is meant to be used multiple times
   in a row, so [free] represents the number of free low bits in [operators].

   @glyph      Raw glyph data from the font
   @width      Width of glyph (1 <= width <= 32)
   @height     Storage height
   @free       Number of free low bits in [operators]
   @operators  VRAM operands

   Returns the number of free bits in [operators] after the operation. If it's
   0, call topti_draw() and reset the operators. If it's neative, call
   topti_draw() then do another pass of topti_split() to recover the missing
   information. */
int topti_split(uint32_t const * glyph, int width, int height, int free,
	uint32_t *operators)
{
	/* Extracts [width] bits on the left of [*glyph] */
	uint32_t glyph_mask = 0xffffffff << (32 - width);
	/* Shifts from the left of [*glyph] to the free bits of [operators] */
	int shift;

	uint32_t data = *glyph++;
	/* Number of bits remaining in [data] */
	int source = 32;

	for(int i = 0; i < height; i++)
	{
		shift = 32 - free;

		/* Read [width] data bits and put them in the operator.
		   * There may not be [width] bits left in [data]; this
		     situation is detected and cared for later. (*1)
		   * There may not be enough space to store [width] bits; this
		     is detected by topti_render(). (*2)
		   * We may have available > 32 as a result of the previous
		     case, so shift carefully. */
		uint32_t line = data & glyph_mask;
		line = (shift >= 0) ? (line >> shift) : (line << -shift);
		operators[i] |= line;

		data <<= width;
		source -= width;

		/* Continue iterating as long as no information is lost */
 		if(source >= 0) continue;

 		/* (*1) Now load a new [data] */
 		uint32_t partial_mask = 0xffffffff << (source + 32);
 		data = *glyph++;
 		shift += source + width;

 		/* shift>=32 means the the information we lost in (*1) does not
 		   fit in the operators, making this a case of (*2). */
 		if(shift < 32)
 		{
 			/* Recover lost bits */
 			uint32_t line = data & partial_mask;
 			line = (shift>=0) ? (line >> shift) : (line << -shift);
 			operators[i] |= line;
 		}

 		data <<= -source;
 		source += 32;
 	}

 	return free - width;
}

/* topti_render(): Render a string on the VRAM
   Combines glyph data onto VRAM operands and blits them efficiently onto the
   VRAM. To write a single character, use a 2-byte string with a NUL.

   @x @y    Target position on VRAM
   @str     Text source
   @f       Font
   @asm_fg  Assembler function for text rendering
   @asm_bg  Assembler function for background rendering */
void topti_render(int x, int y, const char *str, font_t const *f,
	asm_text_t *asm_fg, asm_text_t *asm_bg)
{
	/* Storage height and number of free bits in operators[] */
	int height = f->data_height, free;
	/* Raw glyph data */
	uint32_t const * data = f->prop
		? (void *)(f->sized_data + charset_size(f->charset))
		: f->data;

	/* Basic clipping */
	if(x > 127 || y > 63 || y + height <= 0) return;
	if(y + height > 64) height = 64 - y;

	/* How much we need to skip vertically if we render text at y < 0 */
	int vdisp = 0;
	if(y < 0) vdisp = -y, y = 0;

	/* Operator data and background */
	uint32_t operators[height];
	uint32_t bg[height];
	for(int i = 0; i < height; i++)
	{
		operators[i] = 0;
		bg[i] = 0xffffffff >> (x & 31);
	}

	/* Put an initial offset to the operators to honor the x coordinate */
	free = 32 - (x & 31);
	x >>= 5;
	/* VRAM pointers */
	/* TODO: topti: Support gray vrams */
	uint32_t *v1 = vram + (y << 2) + x;
	uint32_t *v2 = vram + (y << 2) + x;

	/* Pull each character into the operator buffer */
	while(*str)
	{
		int glyph = charset_decode(f->charset, *str++);
		if(glyph < 0) continue;

		int index = topti_offset(f, glyph);

		/* Put glyph data into the operators */
		int width = f->prop ? f->sized_data[glyph] : f->width;
		free = topti_split(data+index, width, height, free, operators);

		/* Potential space after the glyph */
		int space = (*str != 0);
		free -= space;

		if(free > 0) continue;

		/* Once operators are full, update VRAM and start again */

		if(x >= 0)
		{
			asm_bg(v1, v2, bg + vdisp, height - vdisp);
			asm_fg(v1, v2, operators + vdisp, height - vdisp);
		}
		v1++, v2++;
		if(++x >= 4) break;

		for(int i = 0; i < height; i++)
		{
			operators[i] = 0;
			bg[i] = 0xffffffff;
		}
		free += 32;

		/* If information was lost in the split, finish it */

		if(free + space >= 32) continue;

		free += width + space;
		free = topti_split(data+index, width, height, free, operators);
		free -= space;
	}

	/* Put the final longwords */
	if(x >= 0 && x < 4 && free < 32)
	{
		for(int i = 0; i < height; i++) bg[i] &= ~((1 << free) - 1);
		asm_bg(v1, v2, bg + vdisp, height - vdisp);
		asm_fg(v1, v2, operators + vdisp, height - vdisp);
	}
}

/* dsize() - get the width and height of rendered text */
void dsize(const char *str, font_t const * f, int *w, int *h)
{
	if(!f) f = topti_font;
	if(h) *h = f->line_height;
	if(!w) return;

	/* Width for monospaced fonts is easy, unfortunately we still need to
	   compute the length of [str]. Critical applications might do the
	   product themselves to avoid this cost. */
	if(!f->prop)
	{
		int length = 0;
		while(*str++) length++;
		*w = (f->width + 1) * length - 1;
		return;
	}

	/* For proportional fonts, fetch the width of each individual glyphs */
	int width = 0;
	while(*str)
	{
		int glyph = charset_decode(f->charset, *str++);
		if(glyph > 0) width += f->sized_data[glyph] + 1;
	}
	*w = width - 1;
}

/* dtext() - display a string of text */
void dtext(int x, int y, const char *str, color_t fg, color_t bg)
{
	if((uint)fg >= 8 || (uint)bg >= 8) return;
	topti_render(x, y, str, topti_font, topti_asm_text[fg],
		topti_asm_text[bg]);
}
