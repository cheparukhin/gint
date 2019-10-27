#include <gint/display.h>
#include <display/common.h>
#include "topti-asm.h"

/* dtext(): Display a string of text */
GSECTION(".pretext")
void dtext(int x, int y, char const *str, int fg, int bg)
{
	if((uint)fg >= 8 || (uint)bg >= 8) return;
	topti_render(x, y, str, topti_font, topti_asm_text[fg],
		topti_asm_text[bg], gint_vram, gint_vram);
}
