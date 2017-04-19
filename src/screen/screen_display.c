#include <screen.h>
#include <stdint.h>

/*
	screen_display()

	Displays the given vram on the screen. Only bytes can be transferred
	through the screen registers, which is unfortunate because most of the
	vram-related operations use longword-base operations.
*/
void screen_display(const void *ptr)
{
	volatile uint8_t *selector	= (void *)0xb4000000;
	volatile uint8_t *data		= (void *)0xb4010000;
	const uint8_t *vram		= ptr;

	for(int line = 0; line < 64; line++)
	{
		// Setting the x-address register.
		*selector = 4;
		*data = line + 0xc0;

		// Setting Y-Up mode.
		*selector = 1;
		*data = 1;

		// Setting y-address.
		*selector = 4;
		*data = 0;

		// Selecting data write register 7 and sending a line's bytes.
		// We could loop but I suspect it will be faster to iterate.
		*selector = 7;

		/* TODO Manually load the video-ram line into the cache? */

		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;

		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
		*data = *vram++;
	}
}
