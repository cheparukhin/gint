#include <bopti_internals.h>
#include <gray.h>

/*
	gimage()
	Displays a gray image in the video ram.
*/
void gimage(struct Image *img, int x, int y)
{
	int width, layer_size, columns, end;
	int format = img->format, i = 0;
	const unsigned char *data;

	if(img->magic != 0xb7) return;
	op = bopti_op_gray;

	// 'height' refers to a static variable for this file.
	getStructure(img, &width, &height, &layer_size, &data, &columns, &end);

	v1 = gray_lightVRAM();
	v2 = gray_darkVRAM();

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
