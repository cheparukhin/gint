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

/*
	gray_getDelays()
	Returns the gray engine delays.

	@arg	light	Will be set if non-NULL.
	@arg	dark	Will be set if non-NULL.
*/
void gray_getDelays(int *light, int *dark);

/*
	gray_setDelays()
	Changes the gray engine delays. Usually you don't need to call this,
	because the engine has its default values.
	Finding values that give proper grays is quite the hard part of the
	gray engine. Usual values are about 1000, with light being between 75
	and 90% of dark.

	@arg	light	Light gray duration (the lower).
	@arg	dark	Dark gray duration (the higher).
*/
void gray_setDelays(int light, int dark);

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
