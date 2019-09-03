//---
//	gint:core:setup - Installing and unloading the library
//---

#include <gint/gint.h>
#include <gint/drivers.h>
#include <gint/std/string.h>
#include <core/setup.h>
#include <gint/hardware.h>
#include <gint/mpu/intc.h>

/* VBR address, from the linker script */
extern char gint_vbr;
/* System's VBR address */
GBSS static uint32_t system_vbr;
/* Size of exception and TLB handler */
extern char gint_exch_tlbh_size;
/* Driver table */
extern gint_driver_t bdrv, edrv;

//---
//	Context system for gint's core
//---

typedef struct
{
	uint16_t iprs[12];

} GPACKED(2) gint_core_ctx;

/* gint_ctx_save() - save interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_save(gint_core_ctx *ctx)
{
	if(isSH3()) for(int i = 0; i < 8; i++)
		ctx->iprs[i] = *(SH7705_INTC.IPRS[i]);
	else for(int i = 0; i < 12; i++)
		ctx->iprs[i] = SH7305_INTC.IPRS[2 * i];
}

/* gint_ctx_restore() - restore interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_restore(gint_core_ctx *ctx)
{
	if(isSH3()) for(int i = 0; i < 8; i++)
		*(SH7705_INTC.IPRS[i]) = ctx->iprs[i];
	else for(int i = 0; i < 12; i++)
		SH7305_INTC.IPRS[2 * i] = ctx->iprs[i];
}

//---
//	Initialization and unloading
//---

/* System context */
GBSS static gint_core_ctx sys_ctx;

/* lock() - lock interrupts to avoid receiving unsupported signals */
static void lock(void)
{
	/* Just disable everything, drivers will enable what they support */
	if(isSH3()) for(int i = 0; i < 8; i++)
		*(SH7705_INTC.IPRS[i]) = 0x0000;
	else for(int i = 0; i < 12; i++)
		SH7305_INTC.IPRS[2 * i] = 0x0000;
}

/* gint_install() - install and start gint */
void gint_install(void)
{
	/* VBR address, provided by the linker script */
	void *vbr = (void *)&gint_vbr;

	/* Event handler entry points */
	void *inth_entry = isSH3() ? gint_inth_7705 : gint_inth_7305;

	/* Size of the exception and TLB handlers */
	uint32_t exch_tlbh_size = (uint32_t)&gint_exch_tlbh_size;

	/* First save the hardware configuration. This step is crucial because
	   we don't want the system to find out about us directly manipulating
	   the peripheral modules */
	gint_ctx_save(&sys_ctx);

	/* Load the event handler entry points into memory */
	memcpy(vbr + 0x100, gint_exch_tlbh, exch_tlbh_size);
	memcpy(vbr + 0x400, gint_exch_tlbh, exch_tlbh_size);
	memcpy(vbr + 0x600, inth_entry, 32);

	/* Time to switch VBR and roll! */
	system_vbr = gint_setvbr((uint32_t)vbr, lock);
}

/* unlock() - unlock interrupts, restoring system settings */
static void unlock(void)
{
	gint_ctx_restore(&sys_ctx);

	/* Restore all driver settings, but do it in reverse order of loading
	   to honor the dependency system */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(drv->unload) drv->unload();
		if(drv->ctx_restore) drv->ctx_restore(drv->sys_ctx);
	}
}

/* gint_unload() - unload gint and give back control to the system */
void gint_unload(void)
{
	gint_setvbr(system_vbr, unlock);
}
