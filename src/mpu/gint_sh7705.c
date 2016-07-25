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
#include <gint.h>
#include <timer.h>
#include <7705.h>
#include <rtc.h>

#include <stddef.h>

//---
//	Interrupt codes.
//---

#define	IC_RTC_PRI	0x4a0
#define	IC_PINT07	0x700
#define	IC_TMU0_TUNI0	0x400
#define	IC_TMU1_TUNI1	0x420
#define	IC_TMU2_TUNI2	0x440



//---
//	Exception handling.
//---

void gint_int_7705(void)
{
	volatile unsigned int *intevt2 = (unsigned int *)0xa4000000;
	unsigned int code = *intevt2;

	switch(code)
	{
	case IC_RTC_PRI:
		rtc_interrupt();
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

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
volatile void *gint_reg_7705(enum Register reg)
{
	volatile unsigned int *expevt	= (unsigned int *)0xffffffd4;
	volatile unsigned int *mmucr	= (unsigned int *)0xfffffff4;
	volatile unsigned int *tea	= (unsigned int *)0xfffffffc;

	switch(reg)
	{
	case Register_EXPEVT:	return expevt;
	case Register_MMUCR:	return mmucr;
	case Register_TEA:	return tea;
	default:		return NULL;
	}
}

/*
	gint_strerror()
	Returns a string that describe the error set in EXPEVT. This string is
	not platform-dependent.
	Some exception codes represent different errors when invoked inside the
	general exception handler and the TLB error handler. Parameter 'is_tlb'
	should be set to zero for general exception meanings, and anything non-
	zero for TLB error meanings.
*/
const char *gint_strerror_7705(int is_tlb)
{
	volatile unsigned int *expevt = gint_reg_7705(Register_EXPEVT);

	switch(*expevt)
	{
	case 0x1e0: return gint_str[3];
	case 0x0e0: return gint_str[4];
	case 0x040: return gint_str[is_tlb ? 10 : 11];
	case 0x0a0: return gint_str[12];
	case 0x180: return gint_str[6];
	case 0x1a0: return gint_str[7];
	case 0x100: return gint_str[5];
	case 0x060: return gint_str[is_tlb ? 13 : 19];
	case 0x0c0: return gint_str[16];
	case 0x080: return gint_str[20];
	case 0x160: return gint_str[21];
	case 0x5c0: return gint_str[22];
	}
	return gint_str[0];
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
