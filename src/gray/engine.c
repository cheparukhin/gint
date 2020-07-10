//---
//	gint:gray:engine - Core gray engine
//---

#include <gint/defs/types.h>
#include <gint/std/stdlib.h>

#include <gint/drivers/t6k11.h>
#include <gint/gray.h>
#include <gint/display.h>
#include <gint/timer.h>

/* Three additional video RAMS, allocated statically if --static-gray was set
   at configure time, or with malloc() otherwise. */
#ifdef GINT_STATIC_GRAY
GBSS static uint32_t gvrams[3][256];
#endif

/* Four VRAMs: two to draw and two to display */
GBSS static uint32_t *vrams[4];

/* Current VRAM pair used for drawing; the value can either be 0 (draws to
   VRAMs 0 and 1) or 2 (draws to VRAMs 2 and 3). */
static volatile int st = 0;

/* Whether the engine is running. Delays of light and dark frames. */
static int runs = 0;
GBSS static int delays[2];

/* Underlying timer */
#define GRAY_TIMER 0

//---
//	Engine control
//---

/* gray_init(): Engine setup */
GCONSTRUCTOR static void gray_init(void)
{
	/* Here [gint_vram] refers to the standard, monochrome VRAM */
	vrams[0] = gint_vram;

	#ifdef GINT_STATIC_GRAY
	vrams[1] = gvrams[0];
	vrams[2] = gvrams[1];
	vrams[3] = gvrams[2];
	#else
	vrams[1] = malloc(1024);
	vrams[2] = malloc(1024);
	vrams[3] = malloc(1024);
	#endif /* GINT_STATIC_GRAY */

	/* Default delays from Graph 35+E II are different from other models */
	if(gint[HWCALC] == HWCALC_G35PE2)
	{
		delays[0] = 762;
		delays[1] = 1311;
	}
	else
	{
		delays[0] = 898;
		delays[1] = 1350;
	}
}

/* gray_quit(): Engine deinitialization() */
GDESTRUCTOR static void gray_quit(void)
{
	#ifndef GINT_STATIC_GRAY
	free(vrams[1]);
	free(vrams[2]);
	free(vrams[3]);
	#endif /* GINT_STATIC_GRAY */
}

/* gray_int(): Interrupt handler */
int gray_int(void)
{
	t6k11_display(vrams[st ^ 2], 0, 64, 16);
	timer_reload(GRAY_TIMER, delays[(st ^ 3) & 1]);
	st ^= 1;

	return TIMER_CONTINUE;
}

/* gray_start(): Start the gray engine */
void gray_start(void)
{
	#ifndef GINT_STATIC_GRAY
	if(!vrams[1] || !vrams[2] || !vrams[3]) return;
	#endif

	if(runs) return;

	int timer = timer_setup(GRAY_TIMER|TIMER_Pphi_64, delays[0], gray_int);
	if(timer != GRAY_TIMER) return;

	timer_start(GRAY_TIMER);
	st = 0;
	runs = 1;
}

/* gray_stop(): Stop the gray engine */
void gray_stop(void)
{
	timer_stop(GRAY_TIMER);
	runs = 0;
}

/* gray_delays(): Set the gray engine delays */
void gray_delays(uint32_t light, uint32_t dark)
{
	delays[0] = light;
	delays[1] = dark;
}

/* gray_config(): Get the current configuration of the engine */
int gray_config(uint32_t *light, uint32_t *dark)
{
	if(light) *light = delays[0];
	if(dark) *dark = delays[1];

	return runs;
}

/* gupdate(): Push the current VRAMs to the screen */
void gupdate(void)
{
	st ^= 2;
}

/* gvram(): Get the current VRAM pointers */
void gvram(uint32_t **light, uint32_t **dark)
{
	int base = st;

	if(light) *light = vrams[base & 2];
	if(dark) *dark = vrams[base | 1];
}

/* gvraml(): Shorthand to retrieve the current light VRAM pointer */
uint32_t *gvraml(void)
{
	return vrams[st & 2];
}

/* gvramd(): Shorthand to retrieve the current dark VRAM pointer */
uint32_t *gvramd(void)
{
	return vrams[st | 1];
}
