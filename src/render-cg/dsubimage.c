#include <gint/display.h>

/* dsubimage(): Render a section of an image */
void dsubimage(int x, int y, image_t const *img, int left, int top,
	int w, int h, int flags)
{
	int p = img->profile;
	if(p == IMAGE_RGB565 || p == IMAGE_RGB565A)
		return dsubimage_rgb16(x, y, img, left, top, w, h, flags);
	if(p == IMAGE_P8_RGB565 || p == IMAGE_P8_RGB565A)
		return dsubimage_p8(x, y, img, left, top, w, h, flags);
	if(p == IMAGE_P4_RGB565 || p == IMAGE_P4_RGB565A)
		return dsubimage_p4(x, y, img, left, top, w, h, flags);
}
