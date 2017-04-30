#include <modules/timer.h>
#include <modules/rtc.h>
#include <modules/interrupts.h>

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
mod_intc_t	INTC;



//---
//	Initializer
//---

static void mod_init_7705(void)
{
	INTC._7705.iprs.IPRA	= (void *)0xfffffee2;
	INTC._7705.iprs.IPRB	= (void *)0xfffffee4;
	INTC._7705.iprs.IPRC	= (void *)0xa4000016;
	INTC._7705.iprs.IPRD	= (void *)0xa4000018;
	INTC._7705.iprs.IPRE	= (void *)0xa400001a;
	INTC._7705.iprs.IPRF	= (void *)0xa4080000;
	INTC._7705.iprs.IPRG	= (void *)0xa4080002;
	INTC._7705.iprs.IPRH	= (void *)0xa4080004;

	INTC._7705.ICR0		= (void *)0xfffffee0;
	INTC._7705.ICR1		= (void *)0xa4000010;
	INTC._7705.ICR2		= (void *)0xa4000012;
	INTC._7705.PINTER	= (void *)0xa4000014;
	INTC._7705.IRR0		= (void *)0xa4000004;
	INTC._7705.IRR1		= (void *)0xa4000006;
	INTC._7705.IRR2		= (void *)0xa4000008;

	TMU.timers[0]		= (void *)0xfffffe94;
	TMU.timers[1]		= (void *)0xfffffea0;
	TMU.timers[2]		= (void *)0xfffffeac;
	TMU.TSTR		= (void *)0xfffffe92;
	TMU.TCPR2		= (void *)0xfffffeb8;

	RTC.RCR1		= (void *)0xfffffedc;
	RTC.RCR2		= (void *)0xfffffede;
	RTC.time		= (void *)0xfffffec0;
}

static void mod_init_7305(void)
{
	INTC._7305.ICR0		= (void *)0xa4140000;
	INTC._7305.ICR1		= (void *)0xa414001c;
	INTC._7305.INTPRI00	= (void *)0xa4140010;
	INTC._7305.iprs		= (void *)0xa4080000;
	INTC._7305.INTMSK00	= (void *)0xa4140044;
	INTC._7305.masks	= (void *)0xa4080080;
	INTC._7305.INTMSKCLR00	= (void *)0xa4140064;
	INTC._7305.masks_clear	= (void *)0xa40800c0;
	INTC._7305.INTREQ00	= (void *)0xa4140024;
	INTC._7305.USERIMASK	= (void *)0xa4700000;
	INTC._7305.NMIFCR	= (void *)0xa41400c0;

	TMU.timers[0]		= (void *)0xa4490008;
	TMU.timers[1]		= (void *)0xa4490014;
	TMU.timers[2]		= (void *)0xa4490020;
	TMU.TSTR		= (void *)0xa4490004;
	TMU.TCPR2		= NULL;

	RTC.RCR1		= (void *)0xa413fedc;
	RTC.RCR2		= (void *)0xa413fede;
	RTC.time		= (void *)0xa413fec0;
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
