#include <gint/display.h>
#include <display/cg.h>
//#include <gint/drivers/r61524.h>

/* dupdate() - Push the video RAM to the display driver */
void dupdate(void)
{
	r61524_display(gint_vram, 0, 224, 1);

	/* The DMA is still running, so we need to switch VRAMs to avoid
	   overwriting the data which is about to be sent. */
	dvram_switch();
}

/* dupdate_noint() - Push VRAM to the display without interrupts */
void dupdate_noint(void)
{
	r61524_display(gint_vram, 0, 224, 0);
}
