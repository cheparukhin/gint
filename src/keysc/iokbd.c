//---
//	gint:keysc:iokbd - I/O-based keyboard input
//---

#include <defs/types.h>
#include <core/pfc.h>

/* This file is SH7705-only. */
#ifdef FX9860G
#define PFC SH7705_PFC

/* iokbd_delay() - wait a bit so that I/O can keep up
   May use the watchdog timer, but the keyboard driver will need to save it. */
static void iokbd_delay(void)
{
	__asm__(
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
	);

#if 0
	/* Watchdog delay version */
	const int delay = 0xf4;

	/* Disable the watchdog timer interrupt and reset configuration */
	INTC.IPRB.BIT._WDT = 0;
	WDT.WTCSR.WRITE = 0xa500;

	/* Set the delay, input on Pphi / 256 and start counting */
	WDT.WTCNT.WRITE = 0x5a00 | (delay & 0xff);
	WDT.WTCSR.WRITE = 0xa505;
	WDT.WTCSR.WRITE = 0xa585;

	/* Actively wait for overflow, then clear the interrupt flag */
	while((WDT.WTCSR.READ.BYTE & 0x08) == 0);
	WDT.WTCSR.WRITE = 0xa500 | (WDT.WTCSR.READ.BYTE & 0xf7);

	/* Reset configuration, counter, and re-enabled interrupt */
	WDT.WTCSR.WRITE = 0xa500;
	WDT.WTCSR.WRITE = 0x5a00;
	INTC.IPRB.BIT._WDT = GINT_INTP_WDT;
#endif
}

/* iokbd_row() - acquire hit status of a single row from the I/O ports
   @row  Requested row number, 0..9
   Returns 8 bits of key state. */
uint8_t iokbd_row(int row)
{
	if((unsigned)row > 9) return 0x00;
	row ^= 1;

	/* This will enable output (01) on @row, input (10) everywhere else */
	uint16_t ctrl_mask = 0x0003 << ((row & 7) * 2);
	/* Enable output (0) on @row, input (1) everywhere else */
	uint8_t data_mask = ~(1 << (row & 7));

	/* When row < 8, the associated bits are in port B */
	if(row < 8)
	{
		/* Set @row as output in port B; port M is unused */
		PFC.PBCR = 0xaaaa ^ ctrl_mask;
		PFC.PMCR = (PFC.PMCR & 0xff00) | 0x00aa;
		iokbd_delay();

		/* Set @row to 0, everything else to 1 */
		PFC.PBDR = data_mask;
		PFC.PMDR = (PFC.PMDR & 0xf0) | 0x0f;
		iokbd_delay();
	}
	/* When row >= 8, the associated bits are in port M */
	else
	{
		/* Set @row as output in port M; port B is unused */
		PFC.PBCR = 0xaaaa;
		PFC.PMCR = (PFC.PMCR & 0xff00) | (0x00aa ^ ctrl_mask);
		iokbd_delay();

		/* Set @row to 0, everything else to 1 */
		PFC.PBDR = 0xff;
		PFC.PMDR = PFC.PMDR & data_mask;
	}

	/* Now read the input data from the keyboard! */
	uint8_t input = ~PFC.PADR;
	iokbd_delay();

	/* Reset the port configuration. I don't know why the intermediate step
	   is necessary; refer to SimLo's documentation. */
	PFC.PBCR = 0xaaaa;
	PFC.PMCR = (PFC.PMCR & 0xff00) | 0x00aa;
	iokbd_delay();
	PFC.PBCR = 0x5555;
	PFC.PMCR = (PFC.PMCR & 0xff00) | 0x0055;
	iokbd_delay();

	/* Now also reset the data registers. This was forgotten from SimLo's
	   CheckKeyRow() and blows up everything. */
	PFC.PBDR = 0x00;
	PFC.PMDR = PFC.PMDR & 0xf0;

	return input;
}

void iokbd_scan(uint8_t *scan)
{
	/* Scan each row independently; the gain from scanning them altogether
	   is probably not worth it */
	for(int i = 0; i < 12; i++) scan[i] = iokbd_row(i);
}

#endif /* FX9860G */
