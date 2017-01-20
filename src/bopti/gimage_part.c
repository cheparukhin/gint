#include <internals/bopti.h>
#include <internals/display.h>
#include <gray.h>

/*
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

struct Rect
{
	int top, bottom;
	int left, right;
};
struct Rect intersect(struct Rect r1, struct Rect r2)
{
	struct Rect result;
	result.top	= max(r1.top, r2.top);
	result.bottom	= min(r1.bottom, r2.bottom);
	result.left	= max(r1.left, r2.left);
	result.right	= min(r1.right, r2.right);
}
*/

/*
	gimage_part()
	Draws a portion of a gray image, defined by its bounding rectangle.
	Point (left, top) is included, but (left + width, top + height) is
	excluded.
*/
void gimage_part(int x, int y, struct Image *img, int left, int top,
	int width, int height)
{
	if(!img || img->magic != 0x01) return;

	struct Structure s;
	struct Command command;
	int actual_width;
	int format = img->format, i = 0;

	getStructure(img, &s);

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
