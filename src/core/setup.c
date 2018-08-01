//---
//	gint:core:setup - Installing and unloading the library
//---

#include <gint/gint.h>
#include <gint/drivers.h>
#include <core/memory.h>
#include <core/setup.h>
#include <core/intc.h>
#include <core/mpu.h>

/* VBR address, from the linker script */
extern char gint_vbr;
/* System's VBR address */
GBSS static uint32_t system_vbr;
/* Driver table */
extern gint_driver_t bdrv, edrv;

//---
//	Context system for gint's core
//---

typedef struct
{
	uint16_t iprs[12];

} PACKED(2) gint_core_ctx;

/* gint_ctx_save() - save interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_save(gint_core_ctx *ctx)
{
	mpuSwitch(
		/* SH3-based */
		for(int i = 0; i < 8; i++) ctx->iprs[i] = *(INTC3.IPRS[i]),
		/* SH4-based */
		for(int i = 0; i < 12; i++) ctx->iprs[i] = INTC4.IPRS[2 * i]
	);
}

/* gint_ctx_restore() - restore interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_restore(gint_core_ctx *ctx)
{
	mpuSwitch(
		/* SH3-based */
		for(int i = 0; i < 8; i++) *(INTC3.IPRS[i]) = ctx->iprs[i],
		/* SH4-based */
		for(int i = 0; i < 12; i++) INTC4.IPRS[2 * i] = ctx->iprs[i]
	);
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
	mpuSwitch(
		/* SH3-based */
		for(int i = 0; i < 8; i++) *(INTC3.IPRS[i]) = 0x0000,
		/* SH4-based */
		for(int i = 0; i < 12; i++) INTC4.IPRS[2 * i] = 0x0000
	);
}

/* gint_install() - install and start gint */
void gint_install(void)
{
	/* VBR address, provided by the linker script */
	uint32_t vbr = (uint32_t)&gint_vbr;

	/* Event handler entry points */
	void *exch_entry;
	void *inth_entry;

	/* First save the hardware configuration. This step is crucial because
	   we don't want the system to find out about us directly manipulating
	   the peripheral modules */
	gint_ctx_save(&sys_ctx);

	/* Load the event handler entry points into memory */
	/* TODO: Load an exception handler and a TLB miss handler */

	mpuSwitch({
		/* SH3-based */
		extern void exch_entry_7705(void);
		extern void inth_entry_7705(void);
		exch_entry = exch_entry_7705;
		inth_entry = inth_entry_7705;
	},{
		/* SH4-based */
		extern void exch_entry_7305(void);
		extern void inth_entry_7305(void);
		exch_entry = exch_entry_7305;
		inth_entry = inth_entry_7305;
	});

	memcpy((void *)(vbr + 0x100), exch_entry, 32);
	memcpy((void *)(vbr + 0x600), inth_entry, 32);

	/* Time to switch VBR and roll! */
	system_vbr = gint_setvbr(vbr, lock);
}

/* unlock() - unlock interrupts, restoring system settings */
static void unlock(void)
{
	gint_ctx_restore(&sys_ctx);

	/* Restore all driver settings */
	for(gint_driver_t *drv = &bdrv; drv < &edrv; drv++)
	{
		drv->ctx_restore(drv->sys_ctx);
	}
}

/* gint_unload() - unload gint and give back control to the system */
void gint_unload(void)
{
	gint_setvbr(system_vbr, unlock);
}
