//---
//
//	gint core/drawing module: gray
//
//	Runs the gray engine and handles drawing for the dual-buffer system.
//
//---

#ifndef	_GRAY_H
#define	_GRAY_H	1

#include <display.h>

//---
//	Engine control.
//---

/*
	gray_runs()
	Returns 1 if the gray engine is running, 0 otherwise.
*/
int gray_runs(void);

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
	Returns the gray engine delays. Pointers are not set if NULL.
*/
void gray_getDelays(int *light, int *dark);

/*
	gray_setDelays()
	Changes the gray engine delays. Usually you don't need to call this,
	because the engine has its default values.
	Finding values that give proper grays is quite the hard part of the
	gray engine. Usual values are about 1000, with light being between 75
	and 90% of dark.

	Typical values:

	values		stability	stripes		colors
	---------------------------------------------------------
	860, 1298	excellent	worst static	good
	912, 1343	bad		none		very good
	993, 1609	medium		light fast	good       (default)
	---------------------------------------------------------

*/
void gray_setDelays(int light, int dark);



//---
//	Global drawing functions.
//---

/*
	gupdate()
	Swaps the vram buffer sets.
*/
void gupdate(void);

/*
	gclear()
	Clears the video ram.
*/
void gclear(void);

/*
	gclear_area()
	Clears an area of the video ram. End points (x1, y1) and (x2, y2) are
	included.
*/
void gclear_area(int x1, int y1, int x2, int y2);

/*
	greverse_area()
	Reverses an area of the vram. End points (x1, y1) and (x2, y2) are
	included.
*/
void greverse_area(int x1, int y1, int x2, int y2);



//---
//	Local drawing functions.
//---

/*
	gpixel()
	Puts a pixel in the vram.
*/
void gpixel(int x, int y, enum Color color);

/*
	gline()
	Draws a line in the vram. Automatically optimizes special cases.
*/
void gline(int x1, int y1, int x2, int y2, enum Color color);

/*
	gimage()
	Displays a gray image in the vram.
*/
void gimage(struct Image *image, int x, int y);



//---
//	Internal API.
//	Referenced here for documentation purposes only. Do not call.
//--

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
