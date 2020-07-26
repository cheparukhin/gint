//---
//	gint:core:kernel - Installing and unloading the library
//---

#include <gint/gint.h>
#include <gint/drivers.h>
#include <gint/std/string.h>
#include <gint/hardware.h>
#include <gint/mpu/intc.h>

#include "cpu.h"
#include "vbr.h"
#include "drivers.h"
#include "kernel.h"

static void kinit_cpu(void);

//---
//	Context for the CPU and registers not directly managed by a driver
//---

typedef struct
{
	uint32_t VBR;
	uint32_t CPUOPM;
} ctx_t;

/* System context and gint context for the CPU and VBR */
GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(ctx_t *ctx)
{
	if(isSH4()) ctx->CPUOPM = cpu_getCPUOPM();
}
static void ctx_restore(ctx_t *ctx)
{
	if(isSH4()) cpu_setCPUOPM(ctx->CPUOPM);
}

//---
//	Driver control
//---

static void drivers_wait(void)
{
	for driver_asc(d)
	{
		if(d->wait) d->wait();
	}
}

static void drivers_save_and_init(GUNUSED int zero)
{
	/* Initialize the CPU, which is done here instead of in a driver */
	ctx_save(&sys_ctx);
	kinit_cpu();

	for driver_asc(d)
	{
		if(d->ctx_save) d->ctx_save(d->sys_ctx);

		if(isSH3() && d->driver_sh3) d->driver_sh3();
		if(d->init) d->init();
	}
}

static void drivers_restore(int who)
{
	for driver_dsc(d)
	{
		if(d->ctx_restore) d->ctx_restore(who?d->gint_ctx:d->sys_ctx);
	}

	ctx_restore(who ? &gint_ctx : &sys_ctx);
}

static void drivers_switch(int who)
{
	/* Save all drivers in reverse order */
	for driver_dsc(d)
	{
		if(!d->ctx_save || !d->ctx_restore) continue;
		d->ctx_save(who ? d->gint_ctx : d->sys_ctx);
	}
	ctx_save(who ? &gint_ctx : &sys_ctx);

	/* Restore the other context */
	ctx_restore(who ? &sys_ctx : &gint_ctx);
	for driver_asc(d)
	{
		if(!d->ctx_save || !d->ctx_restore) continue;
		d->ctx_restore(who ? d->sys_ctx : d->gint_ctx);
	}
}

//---
//	Initialization and unloading
//---

static void kinit_cpu(void)
{
	if(isSH4()) cpu_setCPUOPM(cpu_getCPUOPM() | 0x00000008);
}

/* kinit(): Install and start gint */
void kinit(void)
{
	/* VBR address, provided by the linker script */
	#ifdef FX9860G
	gint_ctx.VBR = (uint32_t)&gint_vbr_fx9860g;
	#endif
	#ifdef FXCG50
	gint_ctx.VBR = (gint[HWCALC] == HWCALC_FXCG50)
		? (uint32_t)&gint_vbr_fxcg50
		: (uint32_t)&gint_vbr_fxcg20;
	#endif

	/* Event handler entry points */
	void *inth_entry = isSH3() ? gint_inth_7705 : gint_inth_7305;
	uint32_t exch_size = (uint32_t)&gint_exch_size;
	uint32_t tlbh_size = (uint32_t)&gint_tlbh_size;

	/* Load the event handler entry points into memory */
	void *vbr = (void *)gint_ctx.VBR;
	memcpy(vbr + 0x100, gint_exch, exch_size);
	memcpy(vbr + 0x400, gint_tlbh, tlbh_size);
	memcpy(vbr + 0x600, inth_entry, 64);

	/* Take control of the VBR and roll! */
	drivers_wait();
	sys_ctx.VBR = cpu_setVBR(gint_ctx.VBR, drivers_save_and_init, 0);
}

/* Due to dire space restrictions on SH3, event codes that are translated to
   SH4 are further remapped into the VBR space to eliminate gaps and save
   space. Each entry in this table represents a 32-byte block after the
   interrupt handler's entry gate. It shows the SH4 event code whose gate is
   placed on that block (some of gint's SH4 event codes are invented to host
   helper blocks).

   For instance, the 5th block after the entry gate hosts the interrupt handler
   for SH4 event 0x9e0, which is ETMU0 underflow.

   The _inth_remap table in src/core/inth.S combines the SH3-SH4 translation
   with the compact translation, hence its entry for 0xf00 (the SH3 event code
   for ETMU0 underflow) is the offset in this table where 0x9e0 is stored,
   which is 4. */
static const uint16_t sh3_vbr_map[] = {
	0x400,  /* TMU0 underflow */
	0x420,  /* TMU1 underflow */
	0x440,  /* TMU2 underflow */
	0x460,  /* (gint custom: TMU helper) */
	0x9e0,  /* ETMU0 underflow */
	0xc20,  /* ETMU1 underflow (used as helper on SH3) */
	0xc40,  /* ETMU2 underflow (used as helper on SH3) */
	0xc60,  /* (gint custom: ETMU helper) */
	0xaa0,  /* RTC Periodic Interrupt */
	1,	/* (Filler to maintain the gap between 0xaa0 and 0xae0) */
	0xae0,  /* (gint custom: RTC helper) */
	0
};

/* gint_inthandler(): Install interrupt handlers */
void *gint_inthandler(int event_code, const void *handler, size_t size)
{
	void *dest;

	/* Normalize the event code */
	if(event_code < 0x400) return NULL;
	event_code &= ~0x1f;

	/* On SH3, make VBR compact. Use this offset specified in the VBR map
	   above to avoid gaps */
	if(isSH3())
	{
		int index = 0;
		while(sh3_vbr_map[index])
		{
			if((int)sh3_vbr_map[index] == event_code) break;
			index++;
		}

		/* This happens if the event has not beed added to the table,
		   ie. the compact VBR scheme does not support this code */
		if(!sh3_vbr_map[index]) return NULL;

		/* Gates are placed starting at VBR + 0x200 to save space */
		dest = (void *)gint_ctx.VBR + 0x200 + index * 0x20;
	}
	/* On SH4, just use the code as offset */
	else
	{
		/* 0x40 is the size of the entry gate */
		dest = (void *)gint_ctx.VBR + 0x640 + (event_code - 0x400);
	}

	return memcpy(dest, handler, size);
}

/* gint_switch(): Temporarily switch out of gint */
void gint_switch(void (*function)(void))
{
	/* Switch from gint to the OS after a short wait */
	drivers_wait();
	cpu_setVBR(sys_ctx.VBR, drivers_switch, 1);

	if(function) function();

	/* Then switch back to gint once the OS finishes working */
	drivers_wait();
	cpu_setVBR(gint_ctx.VBR, drivers_switch, 0);
}

/* kquit(): Quit gint and give back control to the system */
void kquit(void)
{
	/* Wait for hardware tasks then restore all of the drivers' state and
	   return the VBR space to the OS */
	drivers_wait();
	cpu_setVBR(sys_ctx.VBR, drivers_restore, 0);
}
