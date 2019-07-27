#include <gint/defs/types.h>
#include <gint/display.h>
#include <display/common.h>

/* TODO: These parameters will eventually be specified by the font */
#define CHAR_SPACING 1
#define PROP_SPACING 5

/* dfont(): Set the default font for text rendering */
GSECTION(".pretext")
font_t const *dfont(font_t const * font)
{
	font_t const *old_font = topti_font;

	topti_font = font ? font : gint_default_font;
	return old_font;
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
		*w = (f->width + CHAR_SPACING) * length - CHAR_SPACING;
		return;
	}

	/* For proportional fonts, fetch the width of each individual glyphs */
	int width = 0, c;

	while((c = *str++))
	{
		if(c == ' ')
		{
			width += PROP_SPACING + CHAR_SPACING;
			continue;
		}

		int glyph = charset_decode(f->charset, c);
		if(glyph > 0) width += f->sized_data[glyph] + CHAR_SPACING;
	}
	*w = width - CHAR_SPACING;
}
