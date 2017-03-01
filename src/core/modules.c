#include <modules/timer.h>
#include <stddef.h>
#include <mpu.h>

//---
//	Structure information
//	Here resides most of the platform-dependent register configuration.
//	Module structures are arranged to mask as much as possible hardware
//	differences to the user. When it becomes impossible to do so at
//	compile-time, gint provides functions to ensure that the user does not
//	confront to the hardware directly.
//---

volatile mod_tmu_t TMU;



//---
//	Initializer
//---

static void mod_init_7705(void)
{
	TMU.timers[0]	= (void *)0xfffffe94;
	TMU.timers[1]	= (void *)0xfffffea0;
	TMU.timers[2]	= (void *)0xfffffeac;
	TMU.TSTR	= (void *)0xfffffe92;
	TMU.TCPR2	= (void *)0xfffffeb8;
}

static void mod_init_7305(void)
{
	TMU.timers[0]	= (void *)0xa4490008;
	TMU.timers[1]	= (void *)0xa4490014;
	TMU.timers[2]	= (void *)0xa4490020;
	TMU.TSTR	= (void *)0xa4490004;
	TMU.TCPR2	= NULL;
}

/*
	mod_init()
	Initializes the module data to make register access cross-platform. The
	MPU needs to have been detected or this function will yield wrong
	results.
*/
void mod_init(void)
{
	isSH3() ? mod_init_7705() : mod_init_7305();
}
