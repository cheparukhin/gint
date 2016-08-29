//---
//
//	gint core/drawing module: gray
//
//	Runs the gray engine and handles drawing for the dual-buffer system.
//
//---

#include <gray.h>
#include <screen.h>
#include <timer.h>
#include <mpu.h>

static int internal_vrams[3][256];
static const void *vrams[4];

static int current = 0;
static int delays[2];

static int runs = 0;

#define	GRAY_PRESCALER	TIMER_Po_64



//---
//	Engine control.
//---

/*
	gray_start()
	Starts the gray engine. The control of the screen is transferred to the
	gray engine.
*/
void gray_start(void)
{
	if(runs) return;

	timer_start(TIMER_GRAY, delays[0], GRAY_PRESCALER, gray_interrupt, 0);
	current &= 1;
	runs = 1;
}

/*
	gray_stop()
	Stops the gray engine. The monochrome display system takes control of
	the video ram.
*/
void gray_stop(void)
{
	if(!runs) return;

	timer_stop(TIMER_GRAY);
	runs = 0;

	display_useVRAM(display_getLocalVRAM());
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
//	Engine information.
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
	gray_lightVRAM()
	Returns the module's gray vram address.
*/
void *gray_lightVRAM(void)
{
	return (void *)vrams[~current & 2];
}

/*
	gray_lightVRAM()
	Returns the module's dark vram address.
*/
void *gray_darkVRAM(void)
{
	return (void *)vrams[(~current & 2) | 1];
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



//---
//	Drawing.
//---

/*
	gupdate()
	Swaps the vram buffer sets.
*/
inline void gupdate(void)
{
	current ^= 2;
}



//---
//	Interrupt control and initialization.
//---

/*
	gray_interrupt()
	Answers a timer interrupt. Swaps the buffers.
*/
void gray_interrupt(void)
{
	timer_reload(TIMER_GRAY, delays[(~current) & 1]);
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

	delays[0] = 912;
	delays[1] = 1343;
}
