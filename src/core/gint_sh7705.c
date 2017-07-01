#include <internals/gint.h>
#include <modules/rtc.h>
#include <modules/interrupts.h>
#include <gint.h>
#include <timer.h>
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
//	Register saves, setup, interrupt locks, register restoration.
//---

void gint_save_7705(environment_7705_t *e)
{
	mod_intc_7705_t		*intc	= &INTC._7705;
	mod_intc_ipc_7705_t	*ipc	= &intc->iprs;

	// Saving the interrupt masks from registers IPRA to IPRH.
	e->IPR[0] = ipc->IPRA->word;
	e->IPR[1] = ipc->IPRB->word;
	e->IPR[2] = ipc->IPRC->word;
	e->IPR[3] = ipc->IPRD->word;
	e->IPR[4] = ipc->IPRE->word;
	e->IPR[5] = ipc->IPRF->word;
	e->IPR[6] = ipc->IPRG->word;
	e->IPR[7] = ipc->IPRH->word;

	// Saving RTC registers.
	e->RCR1		= RTC.RCR1->byte;
	e->RCR2		= RTC.RCR2->byte;

	// Saving TMU registers.
	e->TMU0		= *(TMU.timers[0]);
	e->TMU1		= *(TMU.timers[1]);
	e->TMU2		= *(TMU.timers[2]);
	e->TSTR		= TMU.TSTR->byte;

/*	// Saving port data used to access the keyboard.
	e->PACR	 	= PFC.PACR.WORD;
	e->PADR	 	= PA.DR.BYTE;
	e->PBCR	 	= PFC.PBCR.WORD;
	e->PBDR	 	= PB.DR.BYTE;
	e->PMCR	 	= PFC.PMCR.WORD;
	e->PMDR	 	= PM.DR.BYTE;
*/
}

void gint_lock_and_setup_7705(void)
{
	mod_intc_7705_t		*intc	= &INTC._7705;
	mod_intc_ipc_7705_t	*ipc	= &intc->iprs;

	// Disabling everything by default to avoid receiving an interrupt that
	// the handler doesn't handle, which would cause the user program to
	// freeze.
	ipc->IPRA->word		= 0x0000;
	ipc->IPRB->word		= 0x0000;
	ipc->IPRC->word		= 0x0000;
	ipc->IPRD->word		= 0x0000;
	ipc->IPRE->word		= 0x0000;
	ipc->IPRF->word		= 0x0000;
	ipc->IPRG->word		= 0x0000;
	ipc->IPRH->word		= 0x0000;

	// Allowing RTC and timer (which handles keyboard and a whole bunch of
	// other things).
	ipc->IPRA->RTC		= 10;
	ipc->IPRA->TMU0		= 12;
	ipc->IPRA->TMU1		= 12;
	ipc->IPRA->TMU2		= 12;

	// Don't enable RTC periodic signals by default.
	RTC.RCR2->byte		= 0x09;
}

void gint_restore_and_unlock_7705(environment_7705_t *e)
{
	mod_intc_7705_t		*intc	= &INTC._7705;
	mod_intc_ipc_7705_t	*ipc	= &intc->iprs;

	// Restoring the saved states.
	ipc->IPRA->word		= e->IPR[0];
	ipc->IPRB->word		= e->IPR[1];
	ipc->IPRC->word		= e->IPR[2];
	ipc->IPRD->word		= e->IPR[3];
	ipc->IPRE->word		= e->IPR[4];
	ipc->IPRF->word		= e->IPR[5];
	ipc->IPRG->word		= e->IPR[6];
	ipc->IPRH->word		= e->IPR[7];

	// Restoring RTC registers.
	RTC.RCR1->byte		= e->RCR1;
	RTC.RCR2->byte		= e->RCR2;

	// Restoring TMU registers.
	*(TMU.timers[0])	= e->TMU0;
	*(TMU.timers[1])	= e->TMU1;
	*(TMU.timers[2])	= e->TMU2;
	TMU.TSTR->byte		= e->TSTR;

/*	// Restoring keyboard-related I/O port registers.
	PFC.PACR.WORD		= e->PACR;
	PA.DR.BYTE		= e->PADR;
	PFC.PBCR.WORD		= e->PBCR;
	PB.DR.BYTE		= e->PBDR;
	PFC.PMCR.WORD		= e->PMCR;
	PM.DR.BYTE		= e->PMDR;
*/
}
