#include <display.h>

// Program video ram. It resides in BSS section, therefore it is cleared at
// program initialization and stripped from the executable file.
static uint32_t local_vram[256];
uint32_t *vram = local_vram;

/*
	display_getLocalVRAM()
	Returns gint's local video RAM address. Gint does not use the system's
	buffer because it is misaligned. This function always returns the same
	address. Both the display and the gray module heavily use this buffer;
	make sure you don't interfere with them if you access it.
	This function does not necessarily returns the video ram that is
	currently in use; call display_getCurrentVRAM() for this.
*/
inline uint32_t *display_getLocalVRAM(void)
{
	return local_vram;
}

/*
	display_getCurrentVRAM()
	Returns the current monochrome video ram buffer. This function usually
	returns the parameter of the last call to display_useVRAM(), or the
	local vram address (which is default when the library starts).
	The return value of this function is undefined if the gray engine is
	running.
*/
inline uint32_t *display_getCurrentVRAM(void)
{
	return vram;
}

/*
	display_useVRAM()
	Changes the current monochrome video ram address. The argument must be
	a 4-aligned 1024-byte buffer because the library's design requires it.
	This function refuses misaligned buffers but trusts that enough space
	is available; failing to provide enough memory may crash the program.
	This function will most likely have no effect when running the gray
	engine.
*/
inline void display_useVRAM(uint32_t *ptr)
{
	if((intptr_t)ptr & 3) return;
	vram = ptr;
}
