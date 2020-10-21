#include <gint/drivers.h>
#include <gint/hardware.h>
#include <gint/mpu/intc.h>

/* Interrupt controllers */

GDATA3 sh7705_intc_t SH7705_INTC = {
	.IPR = {
		(void *)0xfffffee2, (void *)0xfffffee4,
		(void *)0xa4000016, (void *)0xa4000018, (void *)0xa400001a,
		(void *)0xa4080000, (void *)0xa4080002, (void *)0xa4080004,
	},
	.ICR1 = (void *)0xa4000010,
};

sh7305_intc_t SH7305_INTC = {
	.IPR		= (void *)0xa4080000,
	.MSK		= (void *)0xa4080080,
	.MSKCLR		= (void *)0xa40800c0,
	.USERIMASK	= (void *)0xa4700000,
};

/* Interrupt IPR and IMR positions. The order of entries is as in the named
   list of interrupt signals in <gint/intc.h>. */

/* Friendly names for IPR and IMR register numbers */
enum{ IPRA, IPRB, IPRC, IPRD, IPRE, IPRF, IPRG, IPRH, IPRI, IPRJ, IPRK, IPRL };
enum{ IMR0, IMR1, IMR2, IMR3, IMR4, IMR5, IMR6, IMR7, IMR8, IMR9, IMR10 };
#define _ 0,0

static struct info {
	uint16_t IPR4, IPR4bits, IMR, IMRbits;
	/* Only set if different than IPR4 with IPR4bits */
	uint16_t IPR3, IPR3bits;
} const info[] = {
	/* Standard TMU */
	{ IPRA, 0xf000, IMR4,  0x10, _ },
	{ IPRA, 0x0f00, IMR4,  0x20, _ },
	{ IPRA, 0x00f0, IMR4,  0x40, _ },
	/* ETMU */
	{ IPRJ, 0xf000, IMR6,  0x08, IPRF, 0x000f },
	{ IPRG, 0x0f00, IMR5,  0x02, _ },
	{ IPRG, 0x00f0, IMR5,  0x04, _ },
	{ IPRE, 0x00f0, IMR2,  0x01, _ },
	{ IPRI, 0xf000, IMR6,  0x10, _ },
	{ IPRL, 0xf000, IMR8,  0x02, _ },
	/* DMA */
	{ IPRE, 0xf000, IMR1,  0x01, _ /* Not supported on SH3! */ },
	{ IPRE, 0xf000, IMR1,  0x02, _ },
	{ IPRE, 0xf000, IMR1,  0x04, _ },
	{ IPRE, 0xf000, IMR1,  0x08, _ },
	{ IPRF, 0x0f00, IMR5,  0x10, _ },
	{ IPRF, 0x0f00, IMR5,  0x20, _ },
	{ IPRF, 0x0f00, IMR5,  0x40, _ },
	/* RTC */
	{ IPRK, 0xf000, IMR10, 0x04, IPRA, 0x000f },
	{ IPRK, 0xf000, IMR10, 0x02, IPRA, 0x000f },
	{ IPRK, 0xf000, IMR10, 0x01, IPRA, 0x000f },
	/* SPU */
	{ IPRC, 0x000f, IMR3, 0x04, _ /* Not supported on SH3! */ },
	{ IPRC, 0x000f, IMR4, 0x08, _ },
};


//---
//	Interrupt controller functions
//---

/* intc_priority(): Configure the level of interrupts */
int intc_priority(int intname, int level)
{
	struct info const *i = &info[intname];
	int IPRn = i->IPR4, IPRbits = i->IPR4bits;

	if(isSH3() && i->IPR3bits != 0)
	{
		IPRn = i->IPR3;
		IPRbits = i->IPR3bits;
	}

	/* Bit-shift for the mask */
	int shift = 0;
	while(IPRbits >>= 4) shift += 4;

	uint16_t volatile *IPR;
	IPR = isSH3() ? SH7705_INTC.IPR[IPRn] : &SH7305_INTC.IPR[2*IPRn];

	int oldlevel = (*IPR >> shift) & 0xf;
	*IPR = (*IPR & ~(0xf << shift)) | (level << shift);

	if(isSH4() && level > 0 && i->IMRbits)
	{
		uint8_t volatile *MSKCLR = &SH7305_INTC.MSKCLR->IMR0;
		MSKCLR[4*i->IMR] = i->IMRbits;
	}

	return oldlevel;
}

//---
//	Initialization
//---

static void init(void)
{
	/* Just disable everything, drivers will enable what they support */
	if(isSH3()) for(int i = 0; i < 8; i++)
		*(SH7705_INTC.IPR[i]) = 0x0000;
	else for(int i = 0; i < 12; i++)
		SH7305_INTC.IPR[2 * i] = 0x0000;
}

//---
//	Driver context
//---

typedef struct
{
	uint16_t IPR[12];
	uint8_t MSK[13];
} ctx_t;

GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;

	if(isSH3())
	{
		for(int i = 0; i < 8; i++)
			ctx->IPR[i] = *(SH7705_INTC.IPR[i]);
	}
	else
	{
		for(int i = 0; i < 12; i++)
			ctx->IPR[i] = SH7305_INTC.IPR[2 * i];

		uint8_t *IMR = (void *)SH7305_INTC.MSK;
		for(int i = 0; i < 13; i++, IMR += 4)
			ctx->MSK[i] = *IMR;
	}
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;

	if(isSH3())
	{
		for(int i = 0; i < 8; i++)
			*(SH7705_INTC.IPR[i]) = ctx->IPR[i];
	}
	else
	{
		for(int i = 0; i < 12; i++)
			SH7305_INTC.IPR[2 * i] = ctx->IPR[i];

		/* Setting masks it a bit more involved than reading them */
		uint8_t *IMCR = (void *)SH7305_INTC.MSKCLR;
		uint8_t *IMR  = (void *)SH7305_INTC.MSK;
		for(int i = 0; i < 13; i++, IMR += 4, IMCR += 4)
		{
			*IMCR = 0xff;
			*IMR = ctx->MSK[i];
		}
	}
}

//---
//	Driver structure definition
//---

gint_driver_t drv_intc = {
	.name         = "INTC",
	.init         = init,
	.sys_ctx      = &sys_ctx,
	.gint_ctx     = &gint_ctx,
	.ctx_save     = ctx_save,
	.ctx_restore  = ctx_restore,
};

GINT_DECLARE_DRIVER(0, drv_intc);
