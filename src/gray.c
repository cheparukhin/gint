#include <display.h>
#include <gray.h>
#include <screen.h>

static int internal_vrams[3][256];
const void *vrams[4];

static int current = 0;

/*
	gray_start()
	Starts the gray engine. The control of the screen is transferred to the
	gray engine.
*/
void gray_start(void)
{
}

/*
	gray_stop()
	Stops the gray engine. The monochrome display system takes control of
	the video ram.
*/
void gray_stop(void)
{
	display_useVRAM(display_getLocalVRAM());
}

/*
	gray_lightVRAM()
	Returns the module's gray vram address.
*/
void *gray_lightVRAM(void)
{
	return (void *)vrams[current];
}

/*
	gray_lightVRAM()
	Returns the module's dark vram address.
*/
void *gray_darkVRAM(void)
{
	return (void *)vrams[current + 1];
}

/*
	gray_swap()
	Swaps the vram buffers.
*/
void gray_swap(void)
{
	current = (current + 2) & 3;
}

/*
	gray_interrupt()
	Answers a timer interrupt. Swaps the two buffers.
*/
void gray_interrupt(void)
{
	screen_display(vrams[current]);
	current ^= 1;
}

/*
	gray_init()
	Initializes the gray engine.
*/
void gray_init(void)
{
	vrams[0] = (const void *)display_getLocalVRAM();
	vrams[1] = (const void *)internal_vrams[0];
	vrams[2] = (const void *)internal_vrams[1];
	vrams[3] = (const void *)internal_vrams[2];
}
