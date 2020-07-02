//---
//	gint:drivers:r61524 - Reneses R61524 driver
//
//	This driver is used to control the 16-bit color LCD of the Prizm and
//	fx-CG 50 series.
//---

#ifndef GINT_DRIVERS_R61524
#define GINT_DRIVERS_R61524

#include <gint/defs/types.h>

enum {
	/* Send data through the DMA, return early (triple-buffering) */
	R61524_DMA,
	/* Send data through DMA, wait to return (no interrupts) */
	R61524_DMA_WAIT,
	/* Send data through CPU (slow!) */
	R61524_CPU,
};

/* r61524_display(): Send an image to the display

   This function sends [height] lines of the provided [vram] starting from line
   [start] and going down 396 pixels each line. Three methods are avaiable, the
   default is to use R61524_DMA which is what you almost always want.

   @vram    Source VRAM with a stride of 396*2 bytes
   @start   First line to send
   @height  Number of lines to send
   @method  Transfer method, see above */
void r61524_display(uint16_t *vram, int start, int height, int method);

/* r162524_win_get() and r61524_win_set(): Manipulate the display window

   These functions change the screen rectangle where data is shown. Normally
   gint uses the full screen of 396x224. The system uses a subrectangle of
   384x216.

   These functions don't integrate nicely with gint's drawing API, so if you
   want to use them make sure you know how <gint/display.h> is going to be
   impacted. */
void r61524_win_get(uint16_t *HSA, uint16_t *HEA, uint16_t *VSA,uint16_t *VEA);
void r61524_win_set(uint16_t HSA, uint16_t HEA, uint16_t VSA, uint16_t VEA);

#endif /* GINT_DRIVERS_R61524 */
