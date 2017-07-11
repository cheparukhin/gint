#include <internals/display.h>
#include <gray.h>

/*
	grect()
	Draws a rectangle on the screen. This function can use all colors.
*/
void grect(int x1, int y1, int x2, int y2, color_t operator)
{
	if(operator == color_none) return;
	if(adjustRectangle(&x1, &y1, &x2, &y2)) return;

	uint32_t masks[4];
	getMasks(x1, x2, masks);

	uint32_t *lbase  = gray_lightVRAM() + (y1 << 2);
	uint32_t *lvideo = gray_lightVRAM() + (y2 << 2) + 4;
	uint32_t *dvideo = gray_darkVRAM()  + (y2 << 2) + 4;

	// Doing things in this order will be slower, but man, I can't stand
	// writing that many lines of code for such a simple task. It will be
	// terribly heavy in the binary file...
	while(lvideo > lbase) for(int i = 3; i >= 0; i--) switch(operator)
	{
	case color_white:
		*--lvideo &= ~masks[i];
		*--dvideo &= ~masks[i];
		break;
	case color_light:
		*--lvideo |=  masks[i];
		*--dvideo &= ~masks[i];
		break;
	case color_dark:
		*--lvideo &= ~masks[i];
		*--dvideo |=  masks[i];
		break;
	case color_black:
		*--lvideo |=  masks[i];
		*--dvideo |=  masks[i];
		break;
	case color_none: return;

	case color_invert:
		*--lvideo ^=  masks[i];
		*--dvideo ^=  masks[i];
		break;
	case color_lighten:;
		uint32_t light_1 = *lvideo;
		dvideo--;
		*--lvideo &= *dvideo | ~masks[i];
		*dvideo    = (light_1 | ~masks[i]) & (masks[i] ^ *dvideo);
		break;
	case color_lighten2:
		lvideo--;
		*--dvideo &= *lvideo | ~masks[i];
		*lvideo   &= ~masks[i];
		break;
	case color_darken:;
		uint32_t light_2 = *lvideo;
		dvideo--;
		*--lvideo |= *dvideo & masks[i];
		*dvideo    = (light_2 & masks[i]) | (masks[i] ^ *dvideo);
		break;
	case color_darken2:
		lvideo--;
		*--dvideo |= *lvideo | masks[i];
		*lvideo   |= masks[i];
		break;
	}
}
