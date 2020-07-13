#include <gint/display.h>
#include <gint/drivers/t6k11.h>
#include <display/fx.h>

/* Standard video RAM for fx9860g is 1 bit per pixel */
GSECTION(".bss") static uint32_t fx_vram[256];

/* Here is the definition of the VRAM pointer, exposed in <gint/display.h> */
uint32_t *gint_vram = fx_vram;

/* The current rendering mode */
struct rendering_mode const *dmode = NULL;

/* dupdate() - push the video RAM to the display driver */
void dupdate(void)
{
	if(dmode && dmode->dupdate)
	{
		/* Call the overridden dupdate(), but continue if itreturns
		   non-zero (this is used when stopping the gray engine) */
		int rc = dmode->dupdate();
		if(rc == 0) return;
	}

	t6k11_display(gint_vram, 0, 64, 16);
}
