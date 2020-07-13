#include <gint/display.h>
#include <gint/drivers/r61524.h>
#include "render-cg.h"

/* dupdate() - Push the video RAM to the display driver */
void dupdate(void)
{
	r61524_display(gint_vram, 0, 224, R61524_DMA);

	/* The DMA is still running, so we need to switch VRAMs to avoid
	   overwriting the data which is about to be sent. */
	dvram_switch();
}
