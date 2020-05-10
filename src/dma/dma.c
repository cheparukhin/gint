#include <gint/hardware.h>
#include <gint/mpu/dma.h>
#include <gint/mpu/power.h>
#include <gint/mpu/intc.h>
#include <gint/gint.h>
#include <gint/dma.h>
#include <gint/drivers.h>
#include <gint/clock.h>
#include <gint/display.h>
#include <gint/exc.h>

#define DMA SH7305_DMA
#define INTC SH7305_INTC
#define POWER SH7305_POWER

typedef volatile sh7305_dma_channel_t channel_t;

/* dma_channel(): Get address of a DMA channel */
static channel_t *dma_channel(int channel)
{
	channel_t *addr[6] = {
		&DMA.DMA0, &DMA.DMA1, &DMA.DMA2,
		&DMA.DMA3, &DMA.DMA4, &DMA.DMA5,
	};

	return ((uint)channel >= 6 ? NULL : addr[channel]);
}

/* dma_translate(): Translate virtual address to DMA-suitable form */
static uint32_t dma_translate(void const *address)
{
	uint32_t a = (uint32_t)address;

	/* Preserve RS addresses (as of SH7724 Reference, 11.2.2) */
	if(a >= 0xfd800000 && a < 0xfd800800)
		return a;

	/* Translate virtual addresses to IL memory to physical addresses; the
	   same address is used (as of SH7724 Reference, 10.3.3) */
	if(a >= 0xe5200000 && a < 0xe5204000)
		return a;

	/* First additional on-chip memory area (XRAM) */
	if(a >= 0xe5007000 && a < 0xe5009000)
		return a;

	/* Second on-chip memory area (YRAM) */
	if(a >= 0xe5017000 && a < 0xe5019000)
		return a;

	/* Translate P1 and P2 addresses to ROM and RAM to physical form */
	if(a >= 0x80000000 && a < 0xc0000000)
		return a & 0x1fffffff;

	/* By default: I don't know what this is, let's preserve it */
	return a;
}

//---
//	Driver interface
//---

/* dma_setup(): Setup the DMA in interrupt or no-interrupt mode.
   The first parameters are as for dma_transfer() and dma_transfer_noint(). The
   last parameter indicates whether interrupts should be used.
   Returns non-zero if the DMA is busy or a configuration error occurs. */
static int dma_setup(int channel, dma_size_t size, uint blocks,
	void const *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode,
	int interrupts)
{
	channel_t *ch = dma_channel(channel);
	if(!ch) return 1;

	/* Safety guard: only start a transfer if there's not one running */
	if(ch->CHCR.DE) return 1;

	/* Disable channel and disable the master DMA switch */
	ch->CHCR.DE = 0;
	DMA.OR.DME = 0;

	/* Set DMA source and target address */
	ch->SAR = dma_translate(src);
	ch->DAR = dma_translate(dst);

	/* Set the number of blocks to be transferred */
	ch->TCR = blocks;

	/* Fill in CHCR. Set RS=0100 (auto-request) and the user-provided
	   values for TS (transfer size), DM and SM (address modes) */
	ch->CHCR.lword = 0x00000400;
	ch->CHCR.TS_32 = (size >> 2);
	ch->CHCR.TS_10 = (size & 3);
	ch->CHCR.DM = dst_mode;
	ch->CHCR.SM = src_mode;
	ch->CHCR.IE = !!interrupts;

	/* Prepare DMAOR by enabling the master switch and clearing the
	   blocking flags. */
	DMA.OR.DME = 1;
	DMA.OR.AE = 0;
	DMA.OR.NMIF = 0;

	return 0;
}

/* dma_transfer(): Perform a data transfer */
void dma_transfer(int channel, dma_size_t size, uint blocks,
	void const *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode)
{
	if(dma_setup(channel, size, blocks, src, src_mode, dst, dst_mode, 1))
		return;

	/* Enable channel, starting the DMA transfer. */
	channel_t *ch = dma_channel(channel);
	ch->CHCR.DE = 1;
}

/* dma_transfer_wait(): Wait for a transfer to finish */
void dma_transfer_wait(int channel)
{
	if(channel < 0)
	{
		for(int channel = 0; channel < 6; channel++)
			dma_transfer_wait(channel);
		return;
	}

	channel_t *ch = dma_channel(channel);
	if(!ch) return;

	/* Wait for the channel to be disabled by the interrupt handler.
	   When the source or the destination of the transfer is X, Y or IL
	   memory, refrain from sleeping as this also stops the transfer! */
	int onchip = 0;

	if(ch->SAR >= 0xe5007000 && ch->SAR < 0xe5204000) onchip = 1;
	if(ch->DAR >= 0xe5007000 && ch->DAR < 0xe5204000) onchip = 1;

	while(ch->CHCR.DE)
	{
		if(!onchip) sleep();
	}
}

/* dma_transfer_noint(): Perform a data transfer without interruptions */
void dma_transfer_noint(int channel, dma_size_t size, uint blocks,
	void const *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode)
{
	if(dma_setup(channel, size, blocks, src, src_mode, dst, dst_mode, 0))
		return;

	/* Enable channel, starting the DMA transfer. */
	channel_t *ch = dma_channel(channel);
	ch->CHCR.DE = 1;

	/* Actively wait until the transfer is finished */
	while(!ch->CHCR.TE);

	/* Disable the channel and clear the TE flag. Disable the channel first
	   as clearing the TE flag will allow the transfer to restart */
	ch->CHCR.DE = 0;
	ch->CHCR.TE = 0;

	/* Clear the AE and NMIF status flags */
	DMA.OR.AE = 0;
	DMA.OR.NMIF = 0;
}

//---
//	Initialization
//---

static void init(void)
{
	/* This driver is not implemented on SH3 */
	if(isSH3()) return;

	/* Install the interrupt handler from dma/inth.s */
	int codes[] = { 0x800, 0x820, 0x840, 0x860, 0xb80, 0xba0 };
	extern void inth_dma_te(void);

	for(int i = 0; i < 6; i++)
	{
		/* Install interrupt handler */
		void *h = gint_inthandler(codes[i], inth_dma_te, 32);
		channel_t *ch = dma_channel(i);

		/* Set its CHCR address */
		*(volatile uint32_t **)(h + 24) = &ch->CHCR.lword;
		/* Clear the enable flag */
		ch->CHCR.DE = 0;
	}

	/* Install the address error gate */
	extern void inth_dma_ae(void);
	gint_inthandler(0xbc0, inth_dma_ae, 32);

	/* Set interrupt priority to 3 (IPRE[15..12] for first three channels,
	   IPRF[11..8] for last two and error gate */
	gint_intlevel(16, 3);
	gint_intlevel(21, 3);

	/* Unmask the channel interrupts and the address error */
	INTC.MSKCLR->IMR1 = 0x0f;
	INTC.MSKCLR->IMR5 = 0x70;

	/* Clear blocking flags and enable the master switch */
	DMA.OR.AE = 0;
	DMA.OR.NMIF = 0;
	DMA.OR.DME = 1;

	gint[HWDMA] = HW_LOADED;
}

static void unload(void)
{
	/* Make sure any DMA transfer is finished before leaving the app */
	dma_transfer_wait(-1);
}

//---
//	Context system for this driver
//---

typedef struct
{
	channel_t ch[6];
	uint16_t OR;
	int clock;

} GPACKED(4) ctx_t;

/* One buffer for the system state will go in gint's .bss section */
GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	dma_transfer_wait(-1);

	ctx_t *ctx = buf;

	for(int i = 0; i < 6; i++)
	{
		channel_t *ch = dma_channel(i);
		ctx->ch[i].SAR        = ch->SAR;
		ctx->ch[i].DAR        = ch->DAR;
		ctx->ch[i].TCR        = ch->TCR;
		ctx->ch[i].CHCR.lword = ch->CHCR.lword;
	}

	ctx->OR = DMA.OR.word;

	/* Save the supply status of the DMA0 clock */
	ctx->clock = POWER.MSTPCR0.DMAC0;
}

static void ctx_restore(void *buf)
{
	dma_transfer_wait(-1);

	ctx_t *ctx = buf;

	/* Restore the supply status of the DMA0 clock first. If the DMA was
	   originally on standby, the context is basically trash so we don't
	   want to write that back. If it was originally running, we need to
	   power it up again for the writes to registers to have any effect */
	POWER.MSTPCR0.DMAC0 = ctx->clock;

	for(int i = 0; i < 6; i++)
	{
		channel_t *ch = dma_channel(i);
		ch->SAR        = ctx->ch[i].SAR;
		ch->DAR        = ctx->ch[i].DAR;
		ch->TCR        = ctx->ch[i].TCR;
		ch->CHCR.lword = ctx->ch[i].CHCR.lword;
	}

	DMA.OR.word = ctx->OR;
}

//---
//	Driver structure definition
//---

gint_driver_t drv_dma0 = {
	.name		= "DMA0",
	.init		= init,
	.unload		= unload,
	.sys_ctx	= &sys_ctx,
	.gint_ctx	= &gint_ctx,
	.ctx_save	= ctx_save,
	.ctx_restore	= ctx_restore,
};

GINT_DECLARE_DRIVER(2, drv_dma0);
