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

static uint32_t internal_vrams[3][256];
static uint32_t *vrams[4];

static int current = 0;
static int delays[2];

static int runs = 0;

static timer_t *gray_timer = NULL;



//---
//	Interrupt control and initialization.
//---

/*
	gray_interrupt()
	Answers a timer interrupt. Swaps the buffers.
*/
void gray_interrupt(void)
{
	htimer_reload(timer_gray, delays[(~current) & 1]);

	screen_display(vrams[current]);
	current ^= 1;
}

/*
	gray_init()
	Initializes the gray engine.
*/
__attribute__((constructor)) void gray_init(void)
{
	vrams[0] = display_getLocalVRAM();
	vrams[1] = internal_vrams[0];
	vrams[2] = internal_vrams[1];
	vrams[3] = internal_vrams[2];

	delays[0] = 912;
	delays[1] = 1343;
}



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

	gray_timer = htimer_setup(timer_gray, delays[0], timer_Po_64, 0);
	timer_attach(gray_timer, gray_interrupt, NULL);
	timer_start(gray_timer);

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
	timer_stop(gray_timer);
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
uint32_t *gray_lightVRAM(void)
{
	return vrams[~current & 2];
}

/*
	gray_lightVRAM()
	Returns the module's dark vram address.
*/
uint32_t *gray_darkVRAM(void)
{
	return vrams[(~current & 2) | 1];
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
