#include <internals/bopti.h>
#include <internals/display.h>
#include <gray.h>

/*
	gimage()
	Displays a gray image in the video ram.
*/
void gimage(int x, int y, struct Image *img)
{
	if(!img || img->magic != 0x01) return;

	struct Structure s;
	struct Command command;
	int actual_width;
	int format = img->format, i = 0;

	getStructure(img, &s);

	//---
	//	Adjusting image parameters.
	//---

	if(x + s.width <= 0 || x > 127 || y + s.height <= 0 || y > 63) return;

	command.top	= (y < 0) ? (-y) : (0);
	command.bottom	= (y + s.height > 64) ? (64 - y) : (s.height);
	command.left	= ((x < 0) ? (-x) : (0)) >> 5;
	actual_width	= (x + s.width > 128) ? (128 - x) : (s.width);
	command.right	= ((actual_width + 31) >> 5) - 1;

	command.op	= bopti_op_gray;

	if(x >= 0) getMasks(x, x + actual_width - 1, command.masks);
	else getMasks(0, actual_width + x - 1, command.masks);

	bopti_v1 = gray_lightVRAM();
	bopti_v2 = gray_darkVRAM();

	while(format)
	{
		// Drawing every layer, in order of formats.
		if(format & 1)
		{
			// These members are modified by bopti()!
			command.x = x;
			command.y = y;
			command.channel = (1 << i);

			bopti(s.data, &s, &command);
			s.data += s.layer_size;
		}

		format >>= 1;
		i++;
	}
}
