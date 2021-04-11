//---
//	gint:core:kernel - Installing and unloading the library
//---

#include <gint/gint.h>
#include <gint/drivers.h>
#include <gint/std/string.h>
#include <gint/hardware.h>
#include <gint/mmu.h>
#include <gint/mpu/intc.h>
#include <gint/kmalloc.h>

#include "cpu.h"
#include "vbr.h"
#include "drivers.h"
#include "kernel.h"

static void kinit_cpu(void);

/* Forcefully pull in the INTC driver which gint cannot run without */
extern gint_driver_t drv_intc;
GUNUSED gint_driver_t *gint_required_intc = &drv_intc;

//---
//	Context for the CPU and registers not directly managed by a driver
//---

typedef struct
{
	sr_t SR;
	uint32_t VBR;
	uint32_t CPUOPM;
} ctx_t;

/* System context and gint context for the CPU and VBR */
GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(ctx_t *ctx)
{
	if(isSH4())
	{
		ctx->CPUOPM = cpu_getCPUOPM();
		ctx->SR = cpu_getSR();
	}
}
static void ctx_restore(ctx_t *ctx)
{
	if(isSH4())
	{
		cpu_setCPUOPM(ctx->CPUOPM);
		cpu_setSR(ctx->SR);
	}
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
		if(isSH3() && d->driver_sh3) d->driver_sh3();
		if(d->ctx_save) d->ctx_save(d->sys_ctx);
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
	for driver_dsc(d)
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
	if(isSH4())
	{
		cpu_setCPUOPM(cpu_getCPUOPM() | 0x00000008);

		/* Enable DSP mode on the CPU */
		sr_t SR = cpu_getSR();
		SR.DSP = 1;
		cpu_setSR(SR);
	}
}

/* kinit(): Install and start gint */
void kinit(void)
{
	#ifdef FX9860G
	/* On fx-9860G, VBR is loaded at the end of the user RAM. */
	uint32_t uram_end = (uint32_t)mmu_uram() + mmu_uram_size();
	/* On SH4, stack is at the end of the region, leave 8k */
	if(isSH4()) uram_end -= 0x2000;
	/* VBR size differs with models. On SH3, only 0x600 bytes are used due
	   to the compact scheme. On SH4, 0x1100 bytes are needed to cover the
	   expanded region. */
	uram_end -= (isSH3() ? 0x600 : 0x1100);

	/* There are 0x100 unused bytes at the start of the VBR area */
	gint_ctx.VBR = uram_end - 0x100;
	#endif /* FX9860G */

	#ifdef FXCG50
	/* On fx-CG 50, VBR is loaded at the start of the user RAM. */
	gint_ctx.VBR = (uint32_t)mmu_uram();
	/* All of the user RAM can be used, except for some 16k of stack */
	uint32_t uram_end = (uint32_t)mmu_uram() + mmu_uram_size() - 0x4000;
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
	sys_ctx.VBR = (*cpu_setVBR)(gint_ctx.VBR, drivers_save_and_init, 0);

	/* Initialize memory allocators */
	kmalloc_init();

	/* Create an allocation arena with unused static RAM */
	static kmalloc_arena_t static_ram = { 0 };
	extern uint32_t euram;
	static_ram.name = "_uram";
	static_ram.is_default = isSH4();
	static_ram.start = mmu_uram() + ((uint32_t)&euram - 0x08100000);
	static_ram.end = (void *)uram_end;

	kmalloc_init_arena(&static_ram, true);
	kmalloc_add_arena(&static_ram);
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
	0xd00,  /* ETMU4 underflow (used as helper on SH3) */
	0xd20,  /* (gint custom: ETMU helper) */
	0xd40,  /* (gint custom: ETMU helper) */
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

	/* Prevent writing beyond the end of the VBR space on SH4. Using code
	   0xfc0 into the interrupt handler space (which starts 0x540 bytes
	   into VBR-reserved memory) would reach byte 0x540 + 0xfc0 - 0x400 =
	   0x1100, which is out of gint's reserved VBR area.  */
	if(isSH4() && event_code + size > 0xfc0) return NULL;

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
	(*cpu_setVBR)(sys_ctx.VBR, drivers_switch, 1);

	if(function) function();

	/* Then switch back to gint once the OS finishes working */
	drivers_wait();
	(*cpu_setVBR)(gint_ctx.VBR, drivers_switch, 0);
}

/* kquit(): Quit gint and give back control to the system */
void kquit(void)
{
	/* Wait for hardware tasks then restore all of the drivers' state and
	   return the VBR space to the OS */
	drivers_wait();
	(*cpu_setVBR)(sys_ctx.VBR, drivers_restore, 0);
}
