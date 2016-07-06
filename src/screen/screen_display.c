#include <screen.h>

/*
	screen_display()

	Displays the given vram on the screen. Only bytes can be transferred
	through the screen registers, which is unfortunate because most of the
	vram-related operations use longword-base operations.
*/
void screen_display(const void *ptr)
{
	const char *vram = (const char *)ptr;
	volatile char *selector	= (char *)0xb4000000;
	volatile char *data	= (char *)0xb4010000;
	int line, bytes;

	for(line = 0; line < 64; line++)
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
		*selector = 7;
		for(bytes = 0; bytes < 16; bytes++) *data = *vram++;
	}
}
