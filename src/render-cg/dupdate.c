#include <gint/display.h>
//#include <gint/drivers/r61524.h>

/* dupdate() - Push the video RAM to the display driver */
void dupdate(void)
{
	r61524_display(gint_vram, 0, 224, 1);
}

/* dupdate_noint() - Push VRAM to the display without interrupts */
void dupdate_noint(void)
{
	r61524_display(gint_vram, 0, 224, 0);
}
