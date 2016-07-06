#include <bopti_internals.h>
#include <display.h>

/*
	dimage()
	Displays a monochrome image in the video ram.
*/
void dimage(struct Image *img, int x, int y)
{
	int width, layer_size, columns, end;
	int format = img->format, i = 0;
	const unsigned char *data;

	if(img->magic != 0xb7) return;
	if(img->format != Format_Mono && img->format != Format_MonoAlpha)
		return;
	op = bopti_op_mono;

	// 'height' refers to a static variable for this file.
	getStructure(img, &width, &height, &layer_size, &data, &columns, &end);

	vram = display_getCurrentVRAM();

	while(format)
	{
		// Drawing every layer, in order of formats.
		if(format & 1)
		{
			channel = (1 << i);
			bopti(data, x, y, columns, end);
			data += layer_size;
		}

		format >>= 1;
		i++;
	}

}
