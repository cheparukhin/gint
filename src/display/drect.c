#include <internals/display.h>
#include <display.h>

/*
	drect()
	Draws a rectangle on the screen. This function can use any color which
	is not associated with the gray engine, including the reverse operator.
*/
void drect(int x1, int y1, int x2, int y2, color_t operator)
{
	// Avoid wasting time if the requested operation is invalid here.
	if(operator != color_white && operator != color_black
		&& operator != color_invert) return;
	// Make sure the coordinates are in the right order, and that the
	// requested rectangle crosses the screen.
	if(adjustRectangle(&x1, &y1, &x2, &y2)) return;

	uint32_t masks[4];
	getMasks(x1, x2, masks);

	uint32_t *base	= vram + (y1 << 2);
	uint32_t *video	= vram + (y2 << 2) + 4;

	switch(operator)
	{
	case color_white:
		while(video > base)
		{
			*--video &= ~masks[3];
			*--video &= ~masks[2];
			*--video &= ~masks[1];
			*--video &= ~masks[0];
		}
		break;

	case color_black:
		while(video > base)
		{
			*--video |= masks[3];
			*--video |= masks[2];
			*--video |= masks[1];
			*--video |= masks[0];
		}
		break;

	case color_invert:
		while(video > base)
		{
			*--video ^= masks[3];
			*--video ^= masks[2];
			*--video ^= masks[1];
			*--video ^= masks[0];
		}
		break;

	// Avoid some warnings.
	default: return;
	}
}
