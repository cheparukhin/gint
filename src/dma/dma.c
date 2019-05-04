#include <gint/mpu.h>
#include <gint/mpu/dma.h>
#include <gint/mpu/power.h>
#include <gint/mpu/intc.h>
#include <gint/gint.h>
#include <gint/dma.h>
#include <gint/drivers.h>
#include <gint/clock.h>

#define DMA SH7305_DMA
#define INTC SH7305_INTC
#define POWER SH7305_POWER

//---
//	Driver interface
//---

/* dma_transfer() - Perform a data transfer */
void dma_transfer(dma_size_t size, uint blocks,
	void *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode)
{
	/* Safety guard: only start a transfer if there's not one running */
	if(DMA.DMA0.CHCR.DE) return;

	/* Disable DMA0 and disable the master DMA switch */
	DMA.DMA0.CHCR.DE = 0;
	DMA.OR.DME = 0;

	/* Set DMA source and target address */
	DMA.DMA0.SAR = (uint32_t)src & 0x1fffffff;
	DMA.DMA0.DAR = (uint32_t)dst & 0x1fffffff;

	/* Set the number of blocks to be transferred */
	DMA.DMA0.TCR = blocks;

	/* Fill in CHCR. Set RS=0100 (auto-request) and the user-provided
	   values for TS (transfer size), DM and SM (address modes) */
	DMA.DMA0.CHCR.lword = 0x00000400;
	DMA.DMA0.CHCR.TS_32 = (size >> 2);
	DMA.DMA0.CHCR.TS_10 = (size & 3);
	DMA.DMA0.CHCR.DM = dst_mode;
	DMA.DMA0.CHCR.SM = src_mode;
	DMA.DMA0.CHCR.IE = 1;

	/* Prepare DMAOR by enabling the master switch and clearing the
	   blocking flags. */
	DMA.OR.DME = 1;
	DMA.OR.AE = 0;
	DMA.OR.NMIF = 0;

	/* Enable channel 0, starting the DMA transfer. */
	DMA.DMA0.CHCR.DE = 1;
}

/* dma_transfer_wait() - Wait for a transfer on channel 0 to finish */
void dma_transfer_wait(void)
{
	/* The master switch is cut when the transfer ends */
	while(DMA.OR.DME) sleep();
}

//---
//	Initialization
//---

static void init(void)
{
	/* This driver is not implemented on SH3 */
	if(isSH3()) return;

	/* Install the interrupt handler from dma/inth.s */
	extern void inth_dma_dma0(void);
	gint_inthandler(0x800, inth_dma_dma0, 32);

	/* Set interrupt priority to 3 */
	gint_intlevel(16, 3);

	/* Unmask the DMA0 interrupt */
	INTC.MSKCLR->IMR1 = 0x01;
}

//---
//	Context system for this driver
//---

typedef struct
{
	uint32_t SAR0, DAR0, TCR0, CHCR0;
	uint16_t OR;
	int clock;

} GPACKED(4) ctx_t;

/* One buffer for the system state will go in gint's .bss section */
GBSS static ctx_t sys_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;

	ctx->SAR0  = DMA.DMA0.SAR;
	ctx->DAR0  = DMA.DMA0.DAR;
	ctx->TCR0  = DMA.DMA0.TCR;
	ctx->CHCR0 = DMA.DMA0.CHCR.lword;

	ctx->OR = DMA.OR.word;

	/* Save the supply status of the DMA0 clock */
	ctx->clock = POWER.MSTPCR0.DMAC0;
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;

	DMA.DMA0.SAR = ctx->SAR0;
	DMA.DMA0.DAR = ctx->DAR0;
	DMA.DMA0.TCR = ctx->TCR0;
	DMA.DMA0.CHCR.lword = ctx->CHCR0;

	DMA.OR.word = ctx->OR;

	/* Restore the supply status of the DMA0 clock */
	POWER.MSTPCR0.DMAC0 = ctx->clock;
}

//---
//	Driver structure definition
//---

gint_driver_t drv_dma = {
	.name		= "DMA",
	.init		= init,
	/* Make sure any DMA transfer is finished before leaving the app */
	.unload		= dma_transfer_wait,
	.ctx_size	= sizeof(ctx_t),
	.sys_ctx	= &sys_ctx,
	.ctx_save	= ctx_save,
	.ctx_restore	= ctx_restore,
};

GINT_DECLARE_DRIVER(2, drv_dma);
