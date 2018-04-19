//---
//	gint:core:setup - Installing and unloading the library
//---

#include <gint/gint.h>
#include <core/setup.h>
#include <core/intc.h>
#include <core/mpu.h>

/* Interrupt controllers */

#ifdef FX9860G
GDATA sh7705_intc_t INTC3 = {
	.IPRS = {
		(void *)0xfffffee2, (void *)0xfffffee4,
		(void *)0xa4000016, (void *)0xa4000018, (void *)0xa400001a,
		(void *)0xa4080000, (void *)0xa4080002, (void *)0xa4080004,
	},
	.ICR1		= (void *)0xa4000010,
};
#endif

GDATA sh7305_intc_t INTC4 = {
	.IPRS		= (void *)0xa4080000,
	.MSK		= (void *)0xa4080080,
	.MSKCLR		= (void *)0xa40800c0,
	.USERIMASK	= (void *)0xa4700000,
};

/* VBR address, from the linker script */
extern char gint_vbr;
/* System's VBR address */
GBSS static uint32_t system_vbr;

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
	if(isSH3())
	{
		#ifdef FX9860G
		for(int i = 0; i < 8; i++) ctx->iprs[i] = *(INTC3.IPRS[i]);
		#endif
	}
	else
	{
		for(int i = 0; i < 12; i++) ctx->iprs[i] = INTC4.IPRS[2 * i];
	}
}

/* gint_ctx_restore() - restore interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_restore(gint_core_ctx *ctx)
{
	if(isSH3())
	{
		#ifdef FX9860G
		for(int i = 0; i < 8; i++) *(INTC3.IPRS[i]) = ctx->iprs[i];
		#endif
	}
	else
	{
		for(int i = 0; i < 12; i++) INTC4.IPRS[2 * i] = ctx->iprs[i];
	}
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
	if(isSH3())
	{
		#ifdef FX9860G
		for(int i = 0; i < 8; i++) *(INTC3.IPRS[i]) = 0x0000;
		#endif
	}
	else
	{
		for(int i = 0; i < 12; i++) INTC4.IPRS[2 * i] = 0x0000;
	}
}

/* gint_install() - install and start gint */
void gint_install(void)
{
	/* VBR address, provided by the linker script */
	uint32_t vbr = (uint32_t)&gint_vbr;

	/* Event handler entry points */
	extern void inth_entry_7705(void);
	extern void inth_entry_7305(void);

	/* First save the hardware configuration. This step is crucial because
	   we don't want the system to find out about us directly manipulating
	   the peripheral modules */
	gint_ctx_save(&sys_ctx);

	/* Load the event handler entry points into memory */
	/* TODO: Load an exception handler and a TLB miss handler */

	void *inth_entry = isSH3() ? inth_entry_7705 : inth_entry_7305;
	memcpy((void *)(vbr + 0x600), inth_entry, 32);

	/* Time to switch VBR and roll! */
	system_vbr = gint_setvbr(vbr, lock);
}

/* unlock() - unlock interrupts, restoring system settings */
static void unlock(void)
{
	gint_ctx_restore(&sys_ctx);
}

/* gint_unload() - unload gint and give back control to the system */
void gint_unload(void)
{
	gint_setvbr(system_vbr, unlock);
}
