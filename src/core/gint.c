//---
//	gint:core:gint - Library functions
//---

#include <gint/gint.h>
#include <gint/std/string.h>
#include <gint/hardware.h>
#include <gint/mpu/intc.h>

/* Interrupt controllers */

GDATA3 sh7705_intc_t SH7705_INTC = {
	.IPRS = {
		(void *)0xfffffee2, (void *)0xfffffee4,
		(void *)0xa4000016, (void *)0xa4000018, (void *)0xa400001a,
		(void *)0xa4080000, (void *)0xa4080002, (void *)0xa4080004,
	},
	.ICR1 = (void *)0xa4000010,
};

GDATA sh7305_intc_t SH7305_INTC = {
	.IPRS		= (void *)0xa4080000,
	.MSK		= (void *)0xa4080080,
	.MSKCLR		= (void *)0xa40800c0,
	.USERIMASK	= (void *)0xa4700000,
};

//---
//	Library functions
//---

/* gint_intlevel() - configure the level of interrupts */
int gint_intlevel(int intid, int level)
{
	int shift = (~intid & 0x3) << 2;
	volatile uint16_t *ipr;
	level &= 0xf;

	ipr = isSH3()
	? SH7705_INTC.IPRS[intid >> 2]			/* SH3-based */
	: &SH7305_INTC.IPRS[2 * (intid >> 2)];		/* SH4-based */

	int oldlevel = (*ipr >> shift) & 0xf;
	*ipr = (*ipr & ~(0xf << shift)) | (level << shift);

	return oldlevel;
}

/* gint_inthandler() - configure interrupt handlers */
void *gint_inthandler(int event_code, const void *handler, size_t size)
{
	extern uint32_t gint_vbr;

	/* Normalize the event code */
	event_code -= 0x400;
	event_code &= ~0x1f;

	/* Prevent overriding the entry gate */
	if(event_code < 0) return NULL;

	void *dest = (void *)gint_vbr + event_code + 0x640;
	return memcpy(dest, handler, size);
}
