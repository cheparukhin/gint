#define GINT_NEED_VRAM
#include <gint/display.h>
//#include <gint/drivers/r61524.h>

/* dupdate() - push the video RAM to the display driver */
void dupdate(void)
{
	r61524_display(vram, 0, 224);
}
