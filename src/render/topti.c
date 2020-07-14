#include <gint/defs/types.h>
#include <gint/display.h>

#include "../render/render.h"

/* TODO: These parameters will eventually be specified by the font */
#define CHAR_SPACING 1
#define PROP_SPACING 5

/* dfont(): Set the default font for text rendering */
font_t const *dfont(font_t const * font)
{
	font_t const *old_font = topti_font;

	topti_font = font ? font : gint_default_font;
	return old_font;
}

/* topti_glyph_index(): Obtain the glyph index of a Unicode code point */
int topti_glyph_index(font_t const *f, uint32_t code_point)
{
	int glyph_start = 0;

	for(int i = 0; i < f->block_count; i++)
	{
		int diff = code_point - f->blocks[i].start;
		if(diff >= 0 && diff < f->blocks[i].length)
		{
			return glyph_start + diff;
		}

		glyph_start += f->blocks[i].length;
	}

	return -1;
}

/* topti_offset(): Use a font index to find the location of a glyph */
int topti_offset(font_t const *f, uint glyph)
{
	/* Non-proportional fonts don't need an index */
	if(!f->prop) return glyph * f->storage_size;

	uint8_t const *width = f->glyph_width;

	/* The index gives us the position of all glyphs whose IDs are
	   multiples of 8. Start with a close one and iterate from there. */
	uint g = glyph & ~0x7;
	int offset = f->glyph_index[g >> 3];

	/* Traverse the width array (which is in bits) while converting to
	   longword size */
	while(g < glyph) offset += (width[g++] * f->data_height + 31) >> 5;

	return offset;
}

/* dsize(): Get the width and height of rendered text */
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

		int glyph = topti_glyph_index(f, c);
		if(glyph >= 0) width += f->glyph_width[glyph] + CHAR_SPACING;
	}
	*w = width - CHAR_SPACING;
}
