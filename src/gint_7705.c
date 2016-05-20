#include <gint.h>
#include <timer.h>
#include <keyboard.h>
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
//	Various MPU-dependent procedures.
//---

/*
	gint_setRTCFrequency()
	Sets the RTC interrupt frequency and enables interrupts.

	@arg	frequency
*/
void gint_setRTCFrequency_7705(enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	RTC.RCR2.BYTE = (frequency << 4) | 0x09;
}

/*
	gint_getRTCFrequency()
	Returns the RTC interrupt frequency.

	@return	RTC interrupt frequency.
*/
enum RTCFrequency gint_getRTCFrequency_7705(void)
{
	return (RTC.RCR2.BYTE & 0x70) >> 4;
}



//---
//	Keyboard management.
//---

/*
	kdelay()
	Used to be a low-level sleep using the watchdog, as in the system. This
	way seems OK at least, and it doesn't create column effects as for
	SH7305.
*/
static void kdelay(void)
{
	#define	r4(str)	str str str str

	__asm__
	(
		r4("nop\n\t")
		r4("nop\n\t")
		r4("nop\n\t")
	);

	#undef	r4

	/* Watchdog version.
	const int delay = 0xf4;

	// Disabling the watchdog timer interrupt and resetting the
	// configuration. Setting the delay.
	INTC.IPRB.BIT._WDT = 0;
	WDT.WTCSR.WRITE = 0xa500;
	WDT.WTCNT.WRITE = 0x5a00 | (delay & 0xff);

	// Counting on Po/256.
	WDT.WTCSR.WRITE = 0xa505;
	// Starting the timer (sets again to Po/256).
	WDT.WTCSR.WRITE = 0xa585;

	// Waiting until it overflows (delaying), then clearing the overflow
	// flag.
	while((WDT.WTCSR.READ.BYTE & 0x08) == 0);
	WDT.WTCSR.WRITE = 0xa500 | (WDT.WTCSR.READ.BYTE & 0xf7);

	// Resetting the configuration and the counter.
	WDT.WTCSR.WRITE = 0xa500;
	WDT.WTCSR.WRITE = 0x5a00;

	// Enabling back the watchdog timer interrupt.
	INTC.IPRB.BIT._WDT = GINT_INTP_WDT;
	*/
}

/*
	krow()
	Reads a keyboard row.

	@arg	row	Row to check (0 <= row <= 9).

	@return	Bit-based representation of pressed keys in the checked row.
*/
static int krow(int row)
{
	// '11' on the active row, '00' everywhere else.
	unsigned short smask	= 0x0003 << ((row % 8) * 2);
	// '0' on the active row, '1' everywhere else.
	unsigned char cmask 	= ~(1 << (row % 8));
	// Line results.
	int result		= 0;

	if(row < 0 || row > 9) return 0;

	// Initial configuration.
	PFC.PBCR.WORD = 0xaaaa;
	PFC.PMCR.WORD = (PFC.PMCR.WORD & 0xff00) | 0x0055;
	kdelay();

	if(row < 8)
	{
		// Configuring port B/M as input except for the row to check,
		// which has to be an output. This sets '01' (output) on the
		// active row, '10' (input) everywhere else.
		PFC.PBCR.WORD = 0xaaaa ^ smask;
		PFC.PMCR.WORD = (PFC.PMCR.WORD & 0xff00) | 0x00aa;
		kdelay();

		// Every bit set to 1 except the active row bit.
		PB.DR.BYTE = cmask;
		PM.DR.BYTE = (PM.DR.BYTE & 0xf0) | 0x0f;
		kdelay();
	}
	else
	{
		// The same, but deals with port M.
		PFC.PBCR.WORD = 0xaaaa;
		PFC.PMCR.WORD = ((PFC.PMCR.WORD & 0xff00) | 0x00aa) ^ smask;
		kdelay();

		PB.DR.BYTE = 0xff;
		PM.DR.BYTE = (PM.DR.BYTE & 0xf0) | cmask;
		kdelay();
	}

	// Reading the keyboard row.
	result = ~PA.DR.BYTE;
	kdelay();

	// Re-initializing the port configuration and data.
	PFC.PBCR.WORD = 0xaaaa;
	PFC.PMCR.WORD = (PFC.PMCR.WORD & 0xff00) | 0x00aa;
	kdelay();
	PFC.PBCR.WORD = 0x5555;
	PFC.PMCR.WORD = (PFC.PMCR.WORD & 0xff00) | 0x0055;
	kdelay();
	PB.DR.BYTE = 0x00;
	PM.DR.BYTE &= 0xf0;

	return result;
}

/*
	keyboard_updateState()
	Updates the keyboard state.
*/
void keyboard_updateState_7705(volatile unsigned char *keyboard_state)
{
	int i;

	for(i = 0; i < 10; i++) keyboard_state[i] = krow(i);
}



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
