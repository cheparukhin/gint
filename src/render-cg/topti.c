#define GINT_NEED_VRAM
#include <gint/defs/types.h>
#include <gint/defs/attributes.h>
#include <gint/display.h>
#include <display/common.h>
#include "topti-asm.h"

/* Default font */
extern font_t gint_font8x9;
font_t const * gint_default_font = &gint_font8x9;
font_t const * topti_font = &gint_font8x9;

/* topti_glyph(): Render a glyph on the VRAM
   Prints a glyph naively using word accesses, because for most fonts with a
   small size (including gint's 8x9 font) this will be more efficient than the
   complex logic for longword accesses.

   This function assumes that at least one of [fg] and [bg] is not transparent.

   @vram    Target position on VRAM, adjusted to [top], not adjusted to [left]
   @data    Glyph data
   @left    Left-position of subglyph
   @top     Top-Position of subglyph
   @width   Subglyph width
   @height  Subglyph height
   @dataw   Glyph width
   @fg @bg  Foreground and background colors */
GSECTION(".pretext")
void topti_glyph(uint16_t *vram, uint32_t const * data, int left, int top,
	int width, int height, int dataw, int fg, int bg)
{
	int index = top * dataw + left;

	/* Most common situation: opaque text on transparent background */
	if(bg < 0) topti_glyph_fg(vram + left, data, fg, height, width,
		dataw - width, index);
	/* Full text on opaque background */
	else if(fg >= 0) topti_glyph_fg_bg(vram + left, data, (fg << 16) | bg,
		height, width, dataw - width, index);
	/* Draw background but not text */
	else topti_glyph_bg(vram + left, data, bg, height, width,
		dataw - width, index);
}

GSECTION(".pretext")
void topti_render(int x, int y, char const *str, size_t size, font_t const *f,
	int fg, int bg)
{
	/* Raw glyph data */
	uint32_t const * data;
	if(!f->prop) data = f->data;
	else
	{
		int cs = charset_size(f->charset);
		/* 4-align the result */
		cs += (4 - cs) & 3;
		data = (void *)(f->sized_data + cs);
	}

	/* Storage height, top position within glyph */
	int height = f->data_height, top = 0;

	/* Vertical clipping */
	if(x > 395 || y > 223 || y + height <= 0) return;
	if(y + height > 224) height = 224 - y;
	if(y < 0) top = -y, height += y, y = 0;

	/* Move to top row */
	uint16_t *target = vram + 396 * y;

	/* Character spacing and space waiting to be drawn */
	int space = 1;
	int active_space = 0;

	/* Read each character from the input string */
	while(size--)
	{
		int c = *str++;

		int glyph = charset_decode(f->charset, c);
		if(glyph < 0) continue;

		/* Draw the space if background is opaque */
		int prop_space = (c == ' ' && f->prop) ? 5 : 0;
		if((active_space && bg >= 0) || prop_space)
		{
			active_space += prop_space;
			drect(x, y, x + active_space - 1, y + height - 1, bg);
		}
		x += active_space;
		if(prop_space) { active_space = space; continue; }
		if(x >= 396) break;

		int index = topti_offset(f, glyph);

		/* Compute horizontal intersection between glyph and screen */

		int dataw = f->prop ? f->sized_data[glyph] : f->width;
		int width = dataw, left = 0;

		if(x + dataw <= 0)
		{
			x += dataw;
			active_space = space;
			continue;
		}
		if(x < 0) left = -x, width += x;
		if(x + width > 396) width = 396 - x;

		/* Render glyph */

		topti_glyph(target + x, data + index, left, top, width, height,
			dataw, fg, bg);

		x += dataw;
		active_space = space;
	}
}

/* dtext() - display a string of text */
GSECTION(".pretext")
void dtext(int x, int y, char const *str, int fg, int bg)
{
	topti_render(x, y, str, strlen(str), topti_font, fg, bg);
}
