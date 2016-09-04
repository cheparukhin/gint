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

#include <stddef.h>

//---
//	Interrupt codes.
//---

#define	IC_RTC_PRI	0xaa0
#define	IC_KEYSC	0xbe0
#define	IC_TMU0_TUNI0	0x400
#define	IC_TMU0_TUNI1	0x420
#define	IC_TMU0_TUNI2	0x440



//---
//	Exception handling.
//---

void gint_int_7305(void)
{
	volatile unsigned int *intevt  = (unsigned int *)0xff000028;
	unsigned int code = *intevt;

	switch(code)
	{
	case IC_RTC_PRI:
		rtc_interrupt();
		break;

	case IC_TMU0_TUNI0:
		timer_interrupt(TIMER_TMU0);
		break;

	case IC_TMU0_TUNI1:
		timer_interrupt(TIMER_TMU1);
		break;

	case IC_TMU0_TUNI2:
		timer_interrupt(TIMER_TMU2);
		break;
	}
}

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
volatile void *gint_reg_7305(enum Register reg)
{
	volatile unsigned int *expevt	= (unsigned int *)0xff000024;
	volatile unsigned int *tea	= (unsigned int *)0xff00000c;
	volatile unsigned int *mmucr	= (unsigned int *)0xff000010;

	switch(reg)
	{
	case Register_EXPEVT:	return expevt;
	case Register_TEA:	return tea;
	case Register_MMUCR:	return mmucr;
	default:		return NULL;
	}
}

/*
	gint_strerror()
	Returns a string that describe the error set in EXPEVT. This string is
	not platform-dependent.
*/
const char *gint_strerror_7305(void)
{
	volatile unsigned int *expevt = gint_reg_7305(Register_EXPEVT);

	switch(*expevt)
	{
	case 0x1e0: return gint_str[3];
	case 0x0e0: return gint_str[8];
	case 0x040: return gint_str[14];
	case 0x0a0: return gint_str[17];
	case 0x180: return gint_str[6];
	case 0x1a0: return gint_str[7];
	case 0x100: return gint_str[9];
	case 0x060: return gint_str[15];
	case 0x0c0: return gint_str[18];
	case 0x080: return gint_str[20];
	case 0x160: return gint_str[21];
	}
	return gint_str[0];
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
	INTX.IPRK._RTC		= GINT_INTP_RTC;
	INTX.IPRA.TMU0_0	= GINT_INTP_KEY;
	INTX.IPRA.TMU0_1	= GINT_INTP_GRAY;
	INTX.IPRA.TMU0_2	= GINT_INTP_TIMER;
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
