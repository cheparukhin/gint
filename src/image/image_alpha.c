#include <gint/image.h>

int image_alpha(int format)
{
	switch(format) {
	case IMAGE_RGB565A:
		return 0x0001;
	case IMAGE_P8_RGB565A:
		return -128;
	case IMAGE_P4_RGB565A:
		return 0;
	default:
		return 0x10000;
	}
}
