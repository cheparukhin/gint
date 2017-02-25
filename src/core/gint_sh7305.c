//---
//
//	gint core module: sh7305 interrupt handler
//
//	Of course all the work related to interrupts is heavily platform-
//	dependent. This module handles interrupts and configures the MPU to
//	save and restore the system's configuration when execution ends.
//
//---

#include <internals/gint.h>
#include <internals/rtc.h>
#include <gint.h>
#include <timer.h>
#include <7305.h>
#include <rtc.h>

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
volatile void *gint_reg_7305(gint_register_t reg)
{
	switch(reg)
	{
	case register_tea:	return (void *)0xff00000c;
	case register_mmucr:	return (void *)0xff000010;
	case register_tra:	return (void *)0xff000020;
	case register_expevt:	return (void *)0xff000024;
	case register_intevt:	return (void *)0xff000028;
	default:		return NULL;
	}
}



//---
//	Setup.
//---

static unsigned short ipr[12];
static unsigned char rcr2;
// Saves of the keyboard registers. Could be better.
static unsigned short inj1, inj2, det;
static unsigned char data1, data2, keys, reg;

static void gint_priority_lock_7305(void)
{
	// Saving the current interrupt priorities.
	ipr[0]  = INTX.IPRA.WORD;
	ipr[1]  = INTX.IPRB.WORD;
	ipr[2]  = INTX.IPRC.WORD;
	ipr[3]  = INTX.IPRD.WORD;
	ipr[4]  = INTX.IPRE.WORD;
	ipr[5]  = INTX.IPRF.WORD;
	ipr[6]  = INTX.IPRG.WORD;
	ipr[7]  = INTX.IPRH.WORD;
	ipr[8]  = INTX.IPRI.WORD;
	ipr[9]  = INTX.IPRJ.WORD;
	ipr[10] = INTX.IPRK.WORD;
	ipr[11] = INTX.IPRL.WORD;

	// Disabling everything by default to avoid freezing on non-handled
	// interrupts.
	INTX.IPRA.WORD = 0x0000;
	INTX.IPRB.WORD = 0x0000;
	INTX.IPRC.WORD = 0x0000;
	INTX.IPRD.WORD = 0x0000;
	INTX.IPRE.WORD = 0x0000;
	INTX.IPRF.WORD = 0x0000;
	INTX.IPRG.WORD = 0x0000;
	INTX.IPRH.WORD = 0x0000;
	INTX.IPRI.WORD = 0x0000;
	INTX.IPRJ.WORD = 0x0000;
	INTX.IPRK.WORD = 0x0000;
	INTX.IPRL.WORD = 0x0000;

	// Saving keyboard registers.
	inj1	= *((volatile unsigned short *)0xa4050116);
	data1	= *((volatile unsigned char  *)0xa4050136);
	inj2	= *((volatile unsigned short *)0xa4050118);
	data2	= *((volatile unsigned char  *)0xa4050138);
	det	= *((volatile unsigned short *)0xa405014c);
	keys	= *((volatile unsigned char  *)0xa405016c);
	reg	= *((volatile unsigned char  *)0xa40501c6);

	// Allowing RTC. Keyboard analysis is done regularly using a RTC
	// because SH7305's special KEYSC interface does not allow us to clear
	// the keyboard interrupt flags.
	INTX.IPRK._RTC		= 10;
	INTX.IPRA.TMU0_0	= 12;
	INTX.IPRA.TMU0_1	= 12;
	INTX.IPRA.TMU0_2	= 12;
}

static void gint_priority_unlock_7305(void)
{
	// Restoring the interrupt priorities.
	INTX.IPRA.WORD = ipr[0];
	INTX.IPRB.WORD = ipr[1];
	INTX.IPRC.WORD = ipr[2];
	INTX.IPRD.WORD = ipr[3];
	INTX.IPRE.WORD = ipr[4];
	INTX.IPRF.WORD = ipr[5];
	INTX.IPRG.WORD = ipr[6];
	INTX.IPRH.WORD = ipr[7];
	INTX.IPRI.WORD = ipr[8];
	INTX.IPRJ.WORD = ipr[9];
	INTX.IPRK.WORD = ipr[10];
	INTX.IPRL.WORD = ipr[11];

	// Restoring keyboard registers.
	*((volatile unsigned short *)0xa4050116) = inj1;
	*((volatile unsigned char  *)0xa4050136) = data1;
	*((volatile unsigned short *)0xa4050118) = inj2;
	*((volatile unsigned char  *)0xa4050138) = data2;
	*((volatile unsigned short *)0xa405014c) = det;
	*((volatile unsigned char  *)0xa405016c) = keys;
	*((volatile unsigned char  *)0xa40501c6) = reg;
}

void gint_setup_7305(void)
{
	volatile struct mod_rtc *RTC = RTC_SH7305;

	gint_priority_lock_7305();

	// Saving the RTC configuration.
	rcr2 = RTC->RCR2.BYTE;
	// Disabling RTC interrupts by default.
	RTC->RCR2.BYTE = 0x09;
}

void gint_stop_7305(void)
{
	volatile struct mod_rtc *RTC = RTC_SH7305;

	gint_priority_unlock_7305();

	// Restoring the RTC configuration.
	RTC->RCR2.BYTE = rcr2;
}
