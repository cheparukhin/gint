//---
//	gint:core:setup - Installing and unloading the library
//---

#include <gint/gint.h>
#include <gint/drivers.h>
#include <gint/std/string.h>
#include <core/setup.h>
#include <gint/hardware.h>
#include <gint/mpu/intc.h>
#include <gint/defs/util.h>
#include <gint/display.h>

/* VBR address, from the linker script */
extern char gint_vbr[];
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
	uint8_t masks[13];

} GPACKED(2) gint_core_ctx;

/* gint_ctx_save() - save interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_save(gint_core_ctx *ctx)
{
	if(isSH3())
	{
		for(int i = 0; i < 8; i++)
			ctx->iprs[i] = *(SH7705_INTC.IPRS[i]);
	}
	else
	{
		for(int i = 0; i < 12; i++)
			ctx->iprs[i] = SH7305_INTC.IPRS[2 * i];

		uint8_t *IMR = (void *)SH7305_INTC.MSK;
		for(int i = 0; i < 13; i++, IMR += 4)
			ctx->masks[i] = *IMR;
	}
}

/* gint_ctx_restore() - restore interrupt controller configuration
   @arg  ctx	gint core context object */
static void gint_ctx_restore(gint_core_ctx *ctx)
{
	if(isSH3())
	{
		for(int i = 0; i < 8; i++)
			*(SH7705_INTC.IPRS[i]) = ctx->iprs[i];
	}
	else
	{
		for(int i = 0; i < 12; i++)
			SH7305_INTC.IPRS[2 * i] = ctx->iprs[i];

		/* Setting masks it a bit more involved than reading them */
		uint8_t *IMCR = (void *)SH7305_INTC.MSKCLR;
		uint8_t *IMR  = (void *)SH7305_INTC.MSK;
		for(int i = 0; i < 13; i++, IMR += 4, IMCR += 4)
		{
			*IMCR = 0xff;
			*IMR = ctx->masks[i];
		}
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
	memcpy(vbr + 0x600, inth_entry, 64);

	/* Time to switch VBR and roll! */
	system_vbr = gint_setvbr((uint32_t)vbr, lock);
}

/* unlock() - unlock interrupts, restoring system settings */
static void unlock(void)
{
	/* Restore all driver settings, but do it in reverse order of loading
	   to honor the dependency system */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(drv->ctx_restore) drv->ctx_restore(drv->sys_ctx);
	}

	gint_ctx_restore(&sys_ctx);
}

/* gint_unload() - unload gint and give back control to the system */
void gint_unload(void)
{
	/* First wait for all the drivers to finish their current jobs */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(drv->wait) drv->wait();
	}

	gint_setvbr(system_vbr, unlock);
}

//---
//	Hot switching to operating system
//---

static gint_core_ctx gint_ctx;
extern gint_driver_t bdrv, edrv;

static void gint_switch_out(void)
{
	/* Save all drivers in reverse order */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(!drv->ctx_save || !drv->ctx_restore) continue;
		drv->ctx_save(drv->gint_ctx);
	}
	gint_ctx_save(&gint_ctx);

	/* Restore the system context */
	gint_ctx_restore(&sys_ctx);
	for(gint_driver_t *drv = &bdrv; drv < &edrv; drv++)
	{
		if(!drv->ctx_save || !drv->ctx_restore) continue;
		drv->ctx_restore(drv->sys_ctx);
	}
}

static void gint_switch_in(void)
{
	/* Save system state again */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(!drv->ctx_save || !drv->ctx_restore) continue;
		drv->ctx_save(drv->sys_ctx);
	}
	gint_ctx_save(&sys_ctx);

	/* Restore all drivers to their gint state */
	gint_ctx_restore(&gint_ctx);
	for(gint_driver_t *drv = &bdrv; drv < &edrv; drv++)
	{
		if(!drv->ctx_save || !drv->ctx_restore) continue;
		drv->ctx_restore(drv->gint_ctx);
	}
}

/* gint_switch() - temporarily switch out of gint */
void gint_switch(void (*function)(void))
{
	/* Wait for all the drivers to finish their current jobs */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(drv->wait) drv->wait();
	}

	gint_setvbr(system_vbr, gint_switch_out);
	if(function) function();

	/* Remap all of the ROM */
	extern uint32_t brom, srom;
	volatile void *b = &brom;
	int32_t s = (int32_t)&srom;
	GUNUSED uint8_t x;
	while(s > 0)
	{
		x = *(volatile uint8_t *)b;
		b += 1024;
		s -= 1024;
	}

	/* Wait for the OS to finish working */
	for(gint_driver_t *drv = &edrv; (--drv) >= &bdrv;)
	{
		if(drv->wait) drv->wait();
	}
	gint_setvbr((uint32_t)&gint_vbr, gint_switch_in);
}

int   __Timer_Install(int id, void (*handler)(void), int delay);
int   __Timer_Start(int id);
int   __Timer_Stop(int id);
int   __Timer_Deinstall(int id);
int   __PutKeyCode(int row, int column, int keycode);
int   __GetKeyWait(int *col,int *row,int type,int time,int menu,uint16_t *key);
void  __ClearKeyBuffer(void); /* ? */
void *__GetVRAMAddress(void);
void  __ConfigureStatusArea(int mode);

static int __osmenu_id;

static void __osmenu_handler(void)
{
	__PutKeyCode(0x04, 0x09, 0);

	__Timer_Stop(__osmenu_id);
	__Timer_Deinstall(__osmenu_id);
}

static void __osmenu(void)
{
	__ClearKeyBuffer();

	#ifdef FX9860G
	memcpy(__GetVRAMAddress(), gint_vram, 1024);
	#endif

	#ifdef FXCG50
	/* Unfortunately ineffective (main menu probably reenables it)
	__ConfigureStatusArea(3); */

	/* TODO: Improve copied VRAM behavior in gint_osmenu() on fxcg50 */
	uint16_t *vram1, *vram2;
	dgetvram(&vram1, &vram2);

	uint16_t *dst = __GetVRAMAddress();
	uint16_t *src = (gint_vram == vram1) ? vram2 + 6 : vram1 + 6;

	for(int y = 0; y < 216; y++, dst+=384, src+=396)
	for(int x = 0; x < 384; x++)
	{
		dst[x] = src[x];
	}
	#endif

	/* Mysteriously crashes when coming back; might be useful another time
	   instead of GetKeyWait()
	int C=0x04, R=0x09;
	__SpecialMatrixCodeProcessing(&C, &R); */

	__osmenu_id = __Timer_Install(0, __osmenu_handler, 0 /* ms */);
	if(__osmenu_id <= 0) return;
	__Timer_Start(__osmenu_id);

	int column, row;
	unsigned short keycode;
	__GetKeyWait(&column, &row,
		0 /* KEYWAIT_HALTON_TIMEROFF */,
		1 /* Delay in seconds */,
		0 /* Enable return to main menu */,
		&keycode);
}

/* gint_osmenu() - switch out of gint and call the calculator's main menu */
void gint_osmenu(void)
{
	gint_switch(__osmenu);
}
