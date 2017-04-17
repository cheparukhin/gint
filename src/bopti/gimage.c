#include <internals/bopti.h>
#include <internals/display.h>
#include <gray.h>

/*
	gimage_part()
	Draws a portion of a gray image, defined by its bounding rectangle.
	Point (left, top) is included, but (left + width, top + height) is
	excluded.
*/
void gimage_part(int x, int y, image_t *img, int left, int top, int width,
	int height)
{
	if(!img || img->magic != 0xb7) return;

	structure_t s;
	command_t command;
	int actual_width;
	int format = img->format, i = 0;

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

	command.op	= bopti_op_gray;

	if(x >= 0) getMasks(x, x + actual_width - 1, command.masks);
	else getMasks(0, actual_width + x - 1, command.masks);

	bopti_v1 = gray_lightVRAM();
	bopti_v2 = gray_darkVRAM();

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
	gimage()
	Displays a gray image in the video ram.
*/
void gimage(int x, int y, image_t *img)
{
	gimage_part(x, y, img, 0, 0, -1, -1);
}
