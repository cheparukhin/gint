#include <gint/defs/types.h>
#include <gint/display.h>
#include <display/common.h>

/* dfont(): Set the default font for text rendering */
GSECTION(".pretext")
void dfont(font_t const * font)
{
	topti_font = font ? font : gint_default_font;
}

/* charset_size(): Number of elements in each character set */
GSECTION(".pretext")
int charset_size(enum topti_charset set)
{
	int size[] = { 10, 26, 52, 62, 95, 128 };
	return (uint)set < 6 ? size[set] : -1;
}

/* charset_decode(): Translate ASCII into reduced character sets */
GSECTION(".pretext")
int charset_decode(enum topti_charset set, uint c)
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

/* topti_offset(): Use a font index to find the location of a glyph */
GSECTION(".pretext")
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
