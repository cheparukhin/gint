//---
//
//	gint core module: sh7705 interrupt handler
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
#include <7705.h>
#include <rtc.h>

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
volatile void *gint_reg_7705(gint_register_t reg)
{
	switch(reg)
	{
	case register_intevt:	return (void *)0xa4000000;
	case register_tra:	return (void *)0xffffffd0;
	case register_expevt:	return (void *)0xffffffd4;
	case register_mmucr:	return (void *)0xfffffff4;
	case register_tea:	return (void *)0xfffffffc;
	default:		return NULL;
	}
}



//---
//	Setup.
//---

static unsigned short iprs[8];
static unsigned char rcr2;

static void gint_priority_lock_7705(void)
{
	// Saving the interrupt masks from registers IPRA to IPRH.
	iprs[0] = INTC.IPRA.WORD;
	iprs[1] = INTC.IPRB.WORD;
	iprs[2] = INTX.IPRC.WORD;
	iprs[3] = INTX.IPRD.WORD;
	iprs[4] = INTX.IPRE.WORD;
	iprs[5] = INTX.IPRF.WORD;
	iprs[6] = INTX.IPRG.WORD;
	iprs[7] = INTX.IPRH.WORD;

	// Disabling everything by default to avoid receiving an interrupt that
	// the handler doesn't handle, which would cause the user program to
	// freeze.
	INTC.IPRA.WORD = 0x0000;
	INTC.IPRB.WORD = 0x0000;
	INTX.IPRC.WORD = 0x0000;
	INTX.IPRD.WORD = 0x0000;
	INTX.IPRE.WORD = 0x0000;
	INTX.IPRF.WORD = 0x0000;
	INTX.IPRG.WORD = 0x0000;
	INTX.IPRH.WORD = 0x0000;

	// Allowing RTC, which handles keyboard.
	INTC.IPRA.BIT._RTC	= 10;
	INTC.IPRA.BIT._TMU0	= 12;
	INTC.IPRA.BIT._TMU1	= 12;
	INTC.IPRA.BIT._TMU2	= 12;
}

static void gint_priority_unlock_7705(void)
{
	// Restoring the saved states.
	INTC.IPRA.WORD = iprs[0];
	INTC.IPRB.WORD = iprs[1];
	INTX.IPRC.WORD = iprs[2];
	INTX.IPRD.WORD = iprs[3];
	INTX.IPRE.WORD = iprs[4];
	INTX.IPRF.WORD = iprs[5];
	INTX.IPRG.WORD = iprs[6];
	INTX.IPRH.WORD = iprs[7];
}

void gint_setup_7705(void)
{
	volatile struct mod_rtc *RTC = RTC_SH7705;

	gint_priority_lock_7705();

	// Saving the RTC configuration.
	rcr2 = RTC->RCR2.BYTE;
	// Disabling RTC interrupts by default.
	RTC->RCR2.BYTE = 0x09;
}

void gint_stop_7705(void)
{
	volatile struct mod_rtc *RTC = RTC_SH7705;

	gint_priority_unlock_7705();

	// Restoring the RTC configuration.
	RTC->RCR2.BYTE = rcr2;
}
