#include <gint/display.h>
#include <gint/drivers/t6k11.h>

/* Standard video RAM for fx9860g is 1 bit per pixel */
GSECTION(".bss") static uint32_t fx_vram[256];

/* Here is the definition of the VRAM pointer, exposed in <gint/display.h> */
uint32_t *gint_vram = fx_vram;

/* dupdate() - push the video RAM to the display driver */
void dupdate(void)
{
	t6k11_display(gint_vram, 0, 64, 16);
}

/* dupdate_noint() - Push VRAM to the display without interrupts */
void dupdate_noint(void)
{
	dupdate();
}
