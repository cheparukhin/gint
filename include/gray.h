#ifndef	_GRAY_H
#define	_GRAY_H	1

//---
//	Public API.
//---

/*
	gray_start()
	Starts the gray engine. The control of the screen is transferred to the
	gray engine.
*/
void gray_start(void);

/*
	gray_stop()
	Stops the gray engine. The monochrome display system takes control of
	the video ram.
*/
void gray_stop(void);

/*
	gray_lightVRAM()
	Returns the module's gray vram address.
*/
void *gray_lightVRAM(void);

/*
	gray_lightVRAM()
	Returns the module's dark vram address.
*/
void *gray_darkVRAM(void);

//---
//	Internal API.
//	Referenced here for documentation purposes only. Do not call.
//--

/*
	gray_swap()
	Swaps the vram buffers.
*/
void gray_swap(void);

/*
	gray_interrupt()
	Answers a timer interrupt. Swaps the two buffers.
*/
void gray_interrupt(void)	__attribute__((section(".gint.int")));

/*
	gray_init()
	Initializes the gray engine.
*/
void gray_init(void)		__attribute__((constructor));

#endif	// _GRAY_H
