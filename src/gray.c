//---
//
//	gint core/drawing module: gray
//
//	Runs the gray engine and handles drawing for the dual-buffer system.
//
//---

#include <display.h>
#include <gray.h>
#include <screen.h>
#include <timer.h>

static int internal_vrams[3][256];
const void *vrams[4];

static int current = 0;
static int delays[2];

static int runs = 0;

#define	GRAY_PRESCALER	TIMER_Po_64



//---
//	Engine control.
//---

/*
	gray_runs()
	Returns 1 if the gray engine is running, 0 otherwise.
*/
inline int gray_runs(void)
{
	return runs;
}

/*
	gray_start()
	Starts the gray engine. The control of the screen is transferred to the
	gray engine.
*/
void gray_start(void)
{
	timer_start(TIMER_GRAY, delays[0], GRAY_PRESCALER, gray_interrupt, 0);
	runs = 1;
}

/*
	gray_stop()
	Stops the gray engine. The monochrome display system takes control of
	the video ram.
*/
void gray_stop(void)
{
	timer_stop(TIMER_GRAY);
	runs = 0;

	display_useVRAM(display_getLocalVRAM());
}

/*
	gray_lightVRAM()
	Returns the module's gray vram address.
*/
inline void *gray_lightVRAM(void)
{
	return (void *)vrams[current & 2];
}

/*
	gray_lightVRAM()
	Returns the module's dark vram address.
*/
inline void *gray_darkVRAM(void)
{
	return (void *)vrams[(current & 2) | 1];
}

/*
	gray_getDelays()
	Returns the gray engine delays. Pointers are not set if NULL.
*/
void gray_getDelays(int *light, int *dark)
{
	if(light) *light = delays[0];
	if(dark) *dark = delays[1];
}

/*
	gray_setDelays()
	Changes the gray engine delays.
*/
void gray_setDelays(int light, int dark)
{
	delays[0] = light;
	delays[1] = dark;
}



//---
//	Internal API.
//---

/*
	gray_interrupt()
	Answers a timer interrupt. Swaps the buffers.
*/
void gray_interrupt(void)
{
	timer_reload(TIMER_GRAY, delays[current & 1]);
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

	delays[0] = 900;
	delays[1] = 1000;
}



//---
//	Global drawing functions
//---

/*
	gupdate()
	Swaps the vram buffer sets.
*/
inline void gupdate(void)
{
	current ^= 2;
}

/*
	gclear()
	Clears the video ram.
*/
void gclear(void)
{
	int *v1 = gray_lightVRAM();
	int *v2 = gray_darkVRAM();
	int i;

	for(i = 0; i < 256; i++) v1[i] = v2[i] = 0;
}

/*
	gclear_area()
	Clears an area of the video ram. End points (x1, y1) and (x2, y2) are
	included.
*/
void gclear_area(int x1, int y1, int x2, int y2)
{
	display_useVRAM(gray_lightVRAM());
	dclear_area(x1, y1, x2, y2);
	display_useVRAM(gray_darkVRAM());
	dclear_area(x1, y1, x2, y2);
}

/*
	greverse_area()
	Reverses an area of the vram. End points (x1, y1) and (x2, y2) are
	included.
*/
void greverse_area(int x1, int y1, int x2, int y2)
{
	display_useVRAM(gray_lightVRAM());
	dreverse_area(x1, y1, x2, y2);
	display_useVRAM(gray_darkVRAM());
	dreverse_area(x1, y1, x2, y2);
}



//---
//	Local drawing functions.
//---

/*
	gpixel()
	Puts a pixel in the vram.
*/
void gpixel(int x, int y, enum Color color)
{
	if((unsigned int)x > 127 || (unsigned int)y > 63) return;

	int offset = (y << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	int *v1 = gray_lightVRAM();
	int *v2 = gray_lightVRAM();

	switch(color)
	{
	case Color_White:
		v1[offset] &= ~mask;
		v2[offset] &= ~mask;
		break;

	case Color_Light:
		v1[offset] |= mask;
		v2[offset] &= ~mask;
		break;

	case Color_Dark:
		v1[offset] &= ~mask;
		v2[offset] |= mask;
		break;

	case Color_Black:
		v1[offset] |= mask;
		v2[offset] |= mask;
		break;

	case Color_Invert:
		v1[offset] ^= mask;
		v2[offset] ^= mask;
		break;

	default:
		break;
	}
}

/*
	gline()
	Draws a line in the vram. Automatically optimizes special cases.
*/
void gline(int x1, int y1, int x2, int y2, enum Color color)
{
	enum Color c1, c2;

	if(color == Color_None) return;
	else if(color == Color_Invert) c1 = c2 = Color_Invert;
	else c1 = color & 1, c2 = color >> 1;

	display_useVRAM(gray_lightVRAM());
	dline(x1, y1, x2, y2, c1);
	display_useVRAM(gray_darkVRAM());
	dline(x1, y1, x2, y2, c2);
}
