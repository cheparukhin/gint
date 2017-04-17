#include <internals/bopti.h>
#include <internals/display.h>

/*
	dimage_part()
	Draws a portion of an image, defined by its bounding rectangle.
	Point (left, top) is included, but (left + width, top + height) is
	excluded.
*/
void dimage_part(int x, int y, image_t *img, int left, int top, int width,
	int height)
{
	if(!img || img->magic != 0x01) return;

	structure_t s;
	command_t command;
	int actual_width;
	int format = img->format, i = 0;

	if(format != format_mono && format != format_mono_alpha) return;
	getStructure(img, &s);
	if(width < 0) width = s.width;
	if(height < 0) height = s.height;

	//---
	//	Adjusting the bounding rectangle.
	//---

	// This is what happens when the bounding rectangle overflows from the
	// image...
	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(left + width > s.width) width = s.width - left;
	if(top + height > s.height) height = s.height - top;

	if(x + left + width <= 0 || x > 127 || y + top + height <= 0 || y > 63)
		return;

	command.top	= (y < 0) ? (top - y) : top;
	command.bottom	= top + ((y + height > 64) ? (64 - y) : height);
	command.left	= ((x < 0) ? (left - x) : left) >> 5;
	actual_width	= (x + width > 128) ? (128 - x) : width;
	command.right	= ((left + actual_width + 31) >> 5) - 1;

	command.op	= bopti_op_mono;

	if(x >= 0) getMasks(x, x + actual_width - 1, command.masks);
	else getMasks(0, actual_width + x - 1, command.masks);

	bopti_vram = display_getCurrentVRAM();

	while(format)
	{
		if(format & 1)
		{
			command.x = x - left;
			command.y = y - top;
			command.channel = (1 << i);

			bopti(s.data, &s, &command);
			s.data += s.layer_size;
		}

		format >>= 1;
		i++;
	}
}

/*
	dimage()
	Displays a monochrome image in the video ram.
*/
void dimage(int x, int y, image_t *img)
{
	dimage_part(x, y, img, 0, 0, -1, -1);
}
