#include <bopti_internals.h>
#include <display.h>

/*
	dimage()
	Displays a monochrome image in the video ram.
*/
void dimage(struct Image *img, int x, int y)
{
	if(img->magic != 0xb7) return;

	struct Structure s;
	struct Command command;
	int actual_width;
	int format = img->format, i = 0;

	if(format != Format_Mono && format != Format_MonoAlpha) return;
	getStructure(img, &s);

	//---
	//	Adjusting image parameters.
	//---

	if(x + s.width < 0 || x > 127 || y + s.height < 0 || y > 63) return;

	command.top	= (y < 0) ? (-y) : (0);
	command.bottom	= (y + s.height > 64) ? (64 - y) : (s.height);
	command.left	= ((x < 0) ? (-x) : (0)) >> 5;
	actual_width	= (x + s.width > 128) ? (128 - x) : (s.width);
	command.right	= ((actual_width + 31) >> 5) - 1;

	command.x	= x;
	command.y	= y;
	command.op	= bopti_op_mono;

	getMasks(x, x + actual_width - 1, command.masks);

	vram = display_getCurrentVRAM();

	while(format)
	{
		// Drawing every layer, in order of formats.
		if(format & 1)
		{
			command.channel = (1 << i);
			bopti(s.data, &s, &command);
			s.data += s.layer_size;
		}

		format >>= 1;
		i++;
	}

}
