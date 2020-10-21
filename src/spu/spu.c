#include <gint/mpu/spu.h>
#include <gint/mpu/cpg.h>
#include <gint/mpu/power.h>
#include <gint/drivers.h>
#include <gint/clock.h>
#include <gint/intc.h>

#define SPU   SH7305_SPU
#define DSP0  SH7305_DSP0
#define DSP1  SH7305_DSP1
#define CPG   SH7305_CPG
#define POWER SH7305_POWER

static void init(void)
{
	/* Block SPU interrupts from DSP0, DSP1, and their DMA */
	intc_priority(INTC_SPU_DSP0, 0);
	intc_priority(INTC_SPU_DSP1, 0);

	/* Stop both the SPU and FSI clocks */
	CPG.FSICLKCR.lword = 0x00000103;
	CPG.SPUCLKCR.lword = 0x00000100;
	/* Enable the FSI clock, then the SPU/SPURAM clock */
	CPG.FSICLKCR.CLKSTP = 0;
	CPG.SPUCLKCR.CLKSTP = 0;

	/* Power the clocks through MSTPCR2 */
	POWER.MSTPCR2.FSI_SPU = 0;

	/* Reset the SPU */
	SPU.SPUSRST.RST = 0;
	SPU.SPUSRST.RST = 1;
	sleep_us_spin(1000);

	/* Perform full DSP resets */
	DSP0.DSPCORERST = 1;
	DSP1.DSPCORERST = 1;
	DSP0.DSPRST = 0;
	DSP1.DSPRST = 0;
	sleep_us_spin(1000);
}

int spu_zero(void)
{
	return 0;
}

//---
//	Hardware context
//---

typedef struct
{
	uint32_t PBANKC0, PBANKC1;
	uint32_t XBANKC0, XBANKC1;
} ctx_t;

GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;
	ctx->PBANKC0 = SPU.PBANKC0;
	ctx->PBANKC1 = SPU.PBANKC1;
	ctx->XBANKC0 = SPU.XBANKC0;
	ctx->XBANKC1 = SPU.XBANKC1;
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;
	SPU.PBANKC0 = ctx->PBANKC0;
	SPU.PBANKC1 = ctx->PBANKC1;
	SPU.XBANKC0 = ctx->XBANKC0;
	SPU.XBANKC1 = ctx->XBANKC1;
}

//---
//	Driver structure definition
//---

gint_driver_t drv_spu = {
	.name         = "SPU",
	.init         = init,
	.sys_ctx      = &sys_ctx,
	.gint_ctx     = &gint_ctx,
	.ctx_save     = ctx_save,
	.ctx_restore  = ctx_restore,
};

GINT_DECLARE_DRIVER(3, drv_spu);
