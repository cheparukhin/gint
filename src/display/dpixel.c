#include <internals/display.h>
#include <display.h>

/*
	dpixel()
	Changes a pixel's color in the video ram. The result may depend on the
	current color of the pixel.
*/
void dpixel(size_t x, size_t y, color_t operator)
{
	// Let's be honest, all this module's code *heavily* relies on the
	// screen dimension in the end, so it's not that big a deal.
	if(x > 127 || y > 63) return;

	uint32_t *video	= vram + (y << 2) + (x >> 5);
	uint32_t mask	= 0x80000000 >> (x & 31);

	switch(operator)
	{
	case color_white:	*video &= ~mask;	break;
	case color_black:	*video |= mask;		break;
	case color_invert:	*video ^= mask;		break;
	default: return;
	}
}
