#include <internals/gint.h>
#include <modules/timer.h>
#include <modules/rtc.h>
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
//	Register saves, setup, interrupt locks, register restoration.
//---

void gint_save_7305(environment_7305_t *e)
{
	// Saving interrupt priorities.
	e->IPR[0]	= INTX.IPRA.WORD;
	e->IPR[1]	= INTX.IPRB.WORD;
	e->IPR[2]	= INTX.IPRC.WORD;
	e->IPR[3]	= INTX.IPRD.WORD;
	e->IPR[4]	= INTX.IPRE.WORD;
	e->IPR[5]	= INTX.IPRF.WORD;
	e->IPR[6]	= INTX.IPRG.WORD;
	e->IPR[7]	= INTX.IPRH.WORD;
	e->IPR[8]	= INTX.IPRI.WORD;
	e->IPR[9]	= INTX.IPRJ.WORD;
	e->IPR[10]	= INTX.IPRK.WORD;
	e->IPR[11]	= INTX.IPRL.WORD;

	// Saving RTC registers.
	e->RCR1		= RTC.RCR1->byte;
	e->RCR2		= RTC.RCR2->byte;

	// Saving TMU registers.
	e->TMU0		= *(TMU.timers[0]);
	e->TMU1		= *(TMU.timers[1]);
	e->TMU2		= *(TMU.timers[2]);
	e->TSTR		= TMU.TSTR->byte;

	// Saving port data used to access the keyboard.
	e->PMCR		= *((volatile uint16_t *)0xa4050116);
	e->PMDR		= *((volatile uint8_t  *)0xa4050136);
	e->PNCR		= *((volatile uint16_t *)0xa4050118);
	e->PNDR		= *((volatile uint8_t  *)0xa4050138);
	e->PZCR		= *((volatile uint16_t *)0xa405014c);
	e->PZDR		= *((volatile uint8_t  *)0xa405016c);
	e->key		= *((volatile uint8_t  *)0xa40501c6);
}

void gint_lock_and_setup_7305(void)
{
	// Disabling everything by default to avoid freezing on non-handled
	// interrupts.
	INTX.IPRA.WORD		= 0x0000;
	INTX.IPRB.WORD		= 0x0000;
	INTX.IPRC.WORD		= 0x0000;
	INTX.IPRD.WORD		= 0x0000;
	INTX.IPRE.WORD		= 0x0000;
	INTX.IPRF.WORD		= 0x0000;
	INTX.IPRG.WORD		= 0x0000;
	INTX.IPRH.WORD		= 0x0000;
	INTX.IPRI.WORD		= 0x0000;
	INTX.IPRJ.WORD		= 0x0000;
	INTX.IPRK.WORD		= 0x0000;
	INTX.IPRL.WORD		= 0x0000;

	// Allowing RTC and timer to schedule automatic tasks such as keyboard
	// analysis.
	INTX.IPRK._RTC		= 10;
	INTX.IPRA.TMU0_0	= 12;
	INTX.IPRA.TMU0_1	= 12;
	INTX.IPRA.TMU0_2	= 12;

	// Don't enable the RTC interrupt by default.
	RTC.RCR2->byte = 0x09;
}

void gint_restore_and_unlock_7305(environment_7305_t *e)
{
	// Restoring the interrupt priorities.
	INTX.IPRA.WORD		= e->IPR[0];
	INTX.IPRB.WORD		= e->IPR[1];
	INTX.IPRC.WORD		= e->IPR[2];
	INTX.IPRD.WORD		= e->IPR[3];
	INTX.IPRE.WORD		= e->IPR[4];
	INTX.IPRF.WORD		= e->IPR[5];
	INTX.IPRG.WORD		= e->IPR[6];
	INTX.IPRH.WORD		= e->IPR[7];
	INTX.IPRI.WORD		= e->IPR[8];
	INTX.IPRJ.WORD		= e->IPR[9];
	INTX.IPRK.WORD		= e->IPR[10];
	INTX.IPRL.WORD		= e->IPR[11];

	// Restoring RTC registers.
	RTC.RCR1->byte		= e->RCR1;
	RTC.RCR2->byte		= e->RCR2;

	// Restoring TMU registers.
	*(TMU.timers[0])	= e->TMU0;
	*(TMU.timers[1])	= e->TMU1;
	*(TMU.timers[2])	= e->TMU2;
	TMU.TSTR->byte		= e->TSTR;

	// Restoring keyboard-related I/O port registers. However the backlight
	// pin is in PNDR and we would like the backlight to persist when we
	// leave the application, so we just keep this bit.
	*((volatile uint16_t *)0xa4050116)	= e->PMCR;
	*((volatile uint8_t  *)0xa4050136)	= e->PMDR;
	*((volatile uint16_t *)0xa4050118)	= e->PNCR;
	*((volatile uint8_t  *)0xa4050138)	&= 0x10;
	*((volatile uint8_t  *)0xa4050138)	|= (e->PNDR & ~0x10);
	*((volatile uint16_t *)0xa405014c)	= e->PZCR;
	*((volatile uint8_t  *)0xa405016c)	= e->PZDR;
	*((volatile uint8_t  *)0xa40501c6)	= e->key;
}
