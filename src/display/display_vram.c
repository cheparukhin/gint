#include <display.h>

// Program video ram. It resides in .bss section, therefore it is cleared at
// program initialization and stripped from the executable file.
static int local_vram[256];
int *vram = local_vram;

/*
	display_getLocalVRAM()
	Returns the local video ram address. This function always return the
	same address.
	The buffer returned by this function should not be used directly when
	running the gray engine.
*/
inline void *display_getLocalVRAM(void)
{
	return (void *)local_vram;
}

/*
	display_getCurrentVRAM()
	Returns the current video ram. This function usually returns the
	parameter of the last call to display_useVRAM(), unless the gray engine
	is running (in which case the result is undefined). Returns the local
	vram address by default.
*/
inline void *display_getCurrentVRAM(void)
{
	return (void *)vram;
}

/*
	display_useVRAM()
	Changes the current video ram address. The argument MUST be a 4-
	aligned 1024-byte buffer ; otherwise any drawing operation will crash
	the program.
	This function will most likely have no effect when running the gray
	engine.
*/
inline void display_useVRAM(void *ptr)
{
	vram = (int *)ptr;
}
