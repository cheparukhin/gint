#include <gint/gray.h>
#include <display/common.h>
#include "../render-fx/topti-asm.h"

/* gtext(): Display a string of text */
void gtext(int x, int y, const char *str, int fg, int bg)
{
	if((uint)fg >= 8 || (uint)bg >= 8) return;

	uint32_t *light, *dark;
	gvram(&light, &dark);

	topti_render(x, y, str, topti_font, topti_asm_text[fg],
		topti_asm_text[bg], light, dark);
}
