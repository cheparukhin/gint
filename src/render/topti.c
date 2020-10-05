#include <gint/defs/types.h>
#include <gint/display.h>

#include "../render/render.h"

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

/* topti_utf8_next(): Read the next UTF-8 code point of a string */
uint32_t topti_utf8_next(uint8_t const **str_pointer)
{
	uint8_t const *str = *str_pointer;
	uint8_t lead = *str++;

	/* Skip non-leaders which are invalid as starting bytes */
	while((lead >= 0x80 && lead <= 0xbf) ||
		lead == 0xc0 || lead == 0xc1 || lead == 0xfe || lead == 0xff)
	{
		lead = *str++;
	}

	/* This base case will handle the NUL terminator */
	if(lead <= 0x7f)
	{
		*str_pointer = str;
		return lead;
	}

	uint8_t n2 = (*str++ & 0x3f);
	if(lead <= 0xdf)
	{
		*str_pointer = str;
		return ((lead & 0x1f) << 6) | n2;
	}

	uint8_t n3 = (*str++ & 0x3f);
	if(lead <= 0xef)
	{
		*str_pointer = str;
		return ((lead & 0x0f) << 12) | (n2 << 6) | n3;
	}

	uint8_t n4 = (*str++ & 0x3f);
	if(lead <= 0xf7)
	{
		*str_pointer = str;
		return ((lead & 0x07) << 18) | (n2 << 12) | (n3 << 6) | n4;
	}

	/* It the string is too invalid, force a space and try to continue */
	*str_pointer = str;
	return 0x20;
}

/* dsize(): Get the width and height of rendered text */
void dsize(char const *str_char, font_t const * f, int *w, int *h)
{
	uint8_t const *str = (void *)str_char;
	uint32_t code_point;

	if(!f) f = topti_font;
	if(h) *h = f->line_height;
	if(!w) return;

	/* Width for monospaced fonts is easy, unfortunately we still need to
	   compute the length and group bytes into Unicode code points. */
	if(!f->prop)
	{
		int length = 0;
		while((code_point = topti_utf8_next(&str))) length++;

		*w = (f->width + f->char_spacing) * length - f->char_spacing;
		return;
	}

	/* For proportional fonts, fetch the width of each individual glyphs */
	int width = 0;

	while(1)
	{
		code_point = topti_utf8_next(&str);
		if(!code_point) break;

		int glyph = topti_glyph_index(f, code_point);
		if(glyph < 0) continue;
		width += f->glyph_width[glyph] + f->char_spacing;
	}
	*w = width - f->char_spacing;
}
