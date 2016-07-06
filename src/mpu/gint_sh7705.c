//---
//
//	gint core module: sh7705 interrupt handler
//
//	Of course all the work related to interrupts is heavily platform-
//	dependent. This module handles interrupts and configures the MPU to
//	save and restore the system's configuration when execution ends.
//
//---

#include <gint.h>
#include <timer.h>
#include <7705.h>

//---
//	Interrupt codes.
//---

#define	IC_RTC_PRI	0x4a0
#define	IC_PINT07	0x700
#define	IC_TMU0_TUNI0	0x400
#define	IC_TMU1_TUNI1	0x420
#define	IC_TMU2_TUNI2	0x440



//---
//	Interrupt handler.
//---

void gint_7705(void)
{
	volatile unsigned int *intevt2 = (unsigned int *)0xa4000000;
	unsigned int code = *intevt2;

	switch(code)
	{
	case IC_RTC_PRI:
		RTC.RCR2.BIT.PEF = 0;
		break;

	case IC_TMU0_TUNI0:
		timer_interrupt(TIMER_TMU0);
		break;

	case IC_TMU1_TUNI1:
		timer_interrupt(TIMER_TMU1);
		break;

	case IC_TMU2_TUNI2:
		timer_interrupt(TIMER_TMU2);
		break;
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
	INTC.IPRA.BIT._RTC	= GINT_INTP_RTC;
	INTC.IPRA.BIT._TMU0	= GINT_INTP_KEY;
	INTC.IPRA.BIT._TMU1	= GINT_INTP_GRAY;
	INTC.IPRA.BIT._TMU2	= GINT_INTP_TIMER;
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
	gint_priority_lock_7705();

	// Saving the RTC configuration.
	rcr2 = RTC.RCR2.BYTE;
	// Disabling RTC interrupts by default.
	RTC.RCR2.BYTE = 0x09;
}

void gint_stop_7705(void)
{
	gint_priority_unlock_7705();

	// Restoring the RTC configuration.
	RTC.RCR2.BYTE = rcr2;
}
