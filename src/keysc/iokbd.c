//---
//	gint:keysc:iokbd - I/O-based keyboard input
//---

#include <gint/defs/types.h>
#include <gint/mpu/pfc.h>

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
	uint8_t  volatile *WTCSRr = (void *)0xffffff86;
	uint16_t volatile *WTCSRw = (void *)0xffffff86;
	uint16_t volatile *WTCNTw = (void *)0xffffff84;

	/* Watchdog delay version, each of the values between this and 0xff
	   account for 256 clock cycles on Pphi. This is roughly equivalent to
	   2048 nop, lasting ~70 µs */
	const int delay = 0xf4;

	/* Disable the watchdog timer interrupt and reset configuration */
	*WTCSRw = 0xa500;

	/* Set the delay, input on Pphi / 256 and start counting */
	*WTCNTw = 0x5a00 | (delay & 0xff);
	*WTCSRw = 0xa505;
	*WTCSRw = 0xa585;

	/* Actively wait for overflow, then clear the interrupt flag */
	while((*WTCSRr & 0x08) == 0);
	*WTCSRw = 0xa500 | (*WTCSRr & 0xf7);

	/* Reset configuration, counter, and re-enabled interrupt */
	*WTCSRw = 0xa500;
	*WTCNTw = 0x5a00;
#endif
}

/* iokbd_row() - acquire hit status of a single row from the I/O ports
   @row  Requested row number, 0..9
   Returns 8 bits of key state. */
uint8_t iokbd_row(int row)
{
	if((unsigned)row > 9) return 0x00;
	row ^= 1;

	int orig_PBCR = PFC.PBCR;
	int orig_PMCR = PFC.PMCR;
	int orig_PBDR = PFC.PBDR;
	int orig_PMDR = PFC.PMDR;

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
		iokbd_delay();
	}

	/* Now read the input data from the keyboard! */
	uint8_t input = ~PFC.PADR;
	iokbd_delay();

	/* Reset the port configuration */
	PFC.PBCR = orig_PBCR;
	PFC.PMCR = orig_PMCR;
	iokbd_delay();

	/* Now also reset the data registers. This was forgotten from SimLo's
	   CheckKeyRow() and blows up everything. */
	PFC.PBDR = orig_PBDR;
	PFC.PMDR = orig_PMDR;
	iokbd_delay();

	return input;
}

/* iokbd_scan() - scan ports A/B/M to generate 12 rows of key data */
void iokbd_scan(uint8_t *scan)
{
	/* Scan each row independently; the gain from scanning them altogether
	   is probably not worth it */
	for(int i = 0; i < 12; i++) scan[i] = iokbd_row(i);
}

#endif /* FX9860G */
