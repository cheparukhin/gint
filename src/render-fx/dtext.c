#include <gint/display.h>
#include <display/common.h>
#include "topti-asm.h"

/* dtext_opt(): Display a string of text */
void dtext_opt(int x, int y, int fg, int bg, int halign, int valign,
	char const *str)
{
	if((uint)fg >= 8 || (uint)bg >= 8) return;

	if(halign != DTEXT_LEFT || valign != DTEXT_TOP)
	{
		int w, h;
		dsize(str, topti_font, &w, &h);

		if(halign == DTEXT_RIGHT)  x -= w;
		if(halign == DTEXT_CENTER) x -= ((w+1) >> 1);
		if(valign == DTEXT_BOTTOM) y -= h;
		if(valign == DTEXT_MIDDLE) y -= ((h+1) >> 1);
	}

	topti_render(x, y, str, topti_font, topti_asm_text[fg],
		topti_asm_text[bg], gint_vram, gint_vram);
}

/* dtext(): Simple version of dtext_opt() with defaults */
void dtext(int x, int y, int fg, char const *str)
{
	dtext_opt(x, y, fg, C_NONE, DTEXT_LEFT, DTEXT_TOP, str);
}
