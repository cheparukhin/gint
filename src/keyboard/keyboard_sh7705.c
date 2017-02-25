//---
//
//	gint core module: keyboard analyzer
//
//	Probably the most difficult hardware interaction. There is very few
//	documentation on how the system actually analyzes the keyboard. While
//	disassembling syscalls reveals the following procedure (which was
//	already documented by SimonLothar), there is nothing about the
//	detection problems of the multi-getkey system.
//
//---

#include <keyboard.h>
#include <7705.h>

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
	);

	#undef	r4

	/* Watchdog delay version.
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
void keyboard_updateState_7705(volatile uint8_t *keyboard_state)
{
	for(int i = 0; i < 10; i++) keyboard_state[i] = krow(i);
}
