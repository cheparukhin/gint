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

// Additional video rams used by the gray engine.
static uint32_t internal_vrams[3][256];
static uint32_t *vrams[4];

// Current vram set (0 or 1), delays of the light and dark frames respectively.
static int current = 0;
static int delays[2];

// Is the engine currently running?
static int runs = 0;

// Hardware timer used to run the engine.
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

	/* TODO This may not be very wise considering the fact that the user
	may have specified another monochrome vram address. This raises again
	the idea of a parameter stack. */
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
	gray_currentVRAM()
	Returns the currently displayed video ram (if the engine runs). Used
	internally, but has no interest for the user. You don't want to draw to
	this vram.
*/
uint32_t *gray_currentVRAM(void)
{
	return vrams[current ^ 1];
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
