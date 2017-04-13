#include <modules/timer.h>
#include <modules/rtc.h>

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

mod_tmu_t	TMU;
mod_rtc_t	RTC;



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

	RTC.RCR1	= (void *)0xfffffedc;
	RTC.RCR2	= (void *)0xfffffede;
	RTC.time	= (void *)0xfffffec0;
}

static void mod_init_7305(void)
{
	TMU.timers[0]	= (void *)0xa4490008;
	TMU.timers[1]	= (void *)0xa4490014;
	TMU.timers[2]	= (void *)0xa4490020;
	TMU.TSTR	= (void *)0xa4490004;
	TMU.TCPR2	= NULL;

	RTC.RCR1	= (void *)0xa413fedc;
	RTC.RCR2	= (void *)0xa413fede;
	RTC.time	= (void *)0xa413fec0;
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
