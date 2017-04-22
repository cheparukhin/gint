#include <internals/gint.h>
#include <modules/rtc.h>
#include <modules/interrupts.h>
#include <gint.h>
#include <timer.h>
#include <rtc.h>
#include <7705.h>

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
//	Register saves, setup, interrupt locks, register restoration.
//---

void gint_save_7705(environment_7705_t *e)
{
	// Saving the interrupt masks from registers IPRA to IPRH.
	e->IPR[0] = INTC.IPRA.WORD;
	e->IPR[1] = INTC.IPRB.WORD;
	e->IPR[2] = INTX.IPRC.WORD;
	e->IPR[3] = INTX.IPRD.WORD;
	e->IPR[4] = INTX.IPRE.WORD;
	e->IPR[5] = INTX.IPRF.WORD;
	e->IPR[6] = INTX.IPRG.WORD;
	e->IPR[7] = INTX.IPRH.WORD;

	// Saving RTC registers.
	e->RCR1		= RTC.RCR1->byte;
	e->RCR2		= RTC.RCR2->byte;

	// Saving TMU registers.
	e->TMU0		= *(TMU.timers[0]);
	e->TMU1		= *(TMU.timers[1]);
	e->TMU2		= *(TMU.timers[2]);
	e->TSTR		= TMU.TSTR->byte;

	// Saving port data used to access the keyboard.
	e->PACR	 	= PFC.PACR.WORD;
	e->PADR	 	= PA.DR.BYTE;
	e->PBCR	 	= PFC.PBCR.WORD;
	e->PBDR	 	= PB.DR.BYTE;
	e->PMCR	 	= PFC.PMCR.WORD;
	e->PMDR	 	= PM.DR.BYTE;
}

void gint_lock_and_setup_7705(void)
{
	// Disabling everything by default to avoid receiving an interrupt that
	// the handler doesn't handle, which would cause the user program to
	// freeze.
	INTC.IPRA.WORD		= 0x0000;
	INTC.IPRB.WORD		= 0x0000;
	INTX.IPRC.WORD		= 0x0000;
	INTX.IPRD.WORD		= 0x0000;
	INTX.IPRE.WORD		= 0x0000;
	INTX.IPRF.WORD		= 0x0000;
	INTX.IPRG.WORD		= 0x0000;
	INTX.IPRH.WORD		= 0x0000;

	// Allowing RTC and timer (which handles keyboard and a whole bunch of
	// other things).
	INTC.IPRA.BIT._RTC	= 10;
	INTC.IPRA.BIT._TMU0	= 12;
	INTC.IPRA.BIT._TMU1	= 12;
	INTC.IPRA.BIT._TMU2	= 12;

	// Don't enable RTC periodic signals by default.
	RTC.RCR2->byte		= 0x09;
}

void gint_restore_and_unlock_7705(environment_7705_t *e)
{
	// Restoring the saved states.
	INTC.IPRA.WORD		= e->IPR[0];
	INTC.IPRB.WORD		= e->IPR[1];
	INTX.IPRC.WORD		= e->IPR[2];
	INTX.IPRD.WORD		= e->IPR[3];
	INTX.IPRE.WORD		= e->IPR[4];
	INTX.IPRF.WORD		= e->IPR[5];
	INTX.IPRG.WORD		= e->IPR[6];
	INTX.IPRH.WORD		= e->IPR[7];

	// Restoring RTC registers.
	RTC.RCR1->byte		= e->RCR1;
	RTC.RCR2->byte		= e->RCR2;

	// Restoring TMU registers.
	*(TMU.timers[0])	= e->TMU0;
	*(TMU.timers[1])	= e->TMU1;
	*(TMU.timers[2])	= e->TMU2;
	TMU.TSTR->byte		= e->TSTR;

	// Restoring keyboard-related I/O port registers.
	PFC.PACR.WORD		= e->PACR;
	PA.DR.BYTE		= e->PADR;
	PFC.PBCR.WORD		= e->PBCR;
	PB.DR.BYTE		= e->PBDR;
	PFC.PMCR.WORD		= e->PMCR;
	PM.DR.BYTE		= e->PMDR;
}
