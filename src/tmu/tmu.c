//---
//	gint:tmu - Timer operation
//---

#include <gint/timer.h>
#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/clock.h>

#include <gint/mpu/intc.h>
#include <gint/mpu/tmu.h>

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
//	Timer structures
//---


/* inth_data_t - data storage inside interrupt handlers */
typedef struct
{
	int (*cb)(volatile void *arg);	/* User-provided callback */
	volatile void *arg;		/* Argument for [callback] */
	volatile void *TCR;		/* TCR address for TMU */

} GPACKED(4) inth_data_t;

/* timer_t - all data required to run a single timer */
typedef struct
{
	void *tmu;			/* Address of timer structure */
	inth_data_t *data;		/* Interrupt handler data */
	uint16_t event;			/* Interrupt event code */
} timer_t;

//---
//	Driver storage
//---

/* This is the description of the structure on SH4. SH3-based fx9860g models,
   which are already very rare, will adapt the values in init functions */
GDATA static timer_t timers[9] = {
	{ (void *)0xa4490008, NULL, 0x400 },
	{ (void *)0xa4490014, NULL, 0x420 },
	{ (void *)0xa4490020, NULL, 0x440 },
	{ (void *)0xa44d0030, NULL, 0x9e0 },
	{ (void *)0xa44d0050, NULL, 0xc20 },
	{ (void *)0xa44d0070, NULL, 0xc40 },
	{ (void *)0xa44d0090, NULL, 0x900 },
	{ (void *)0xa44d00b0, NULL, 0xd00 },
	{ (void *)0xa44d00d0, NULL, 0xfa0 },
};

/* TSTR register for standard timers */
GDATA static volatile uint8_t *TSTR = (void *)0xa4490004;

//---
//	Timer API
//---

/* timer_setup() - set up a timer */
int timer_setup(int id, uint32_t delay, timer_input_t clock,
	int (*callback)(volatile void *arg), volatile void *arg)
{
	/* We need to distinguish normal and extra timers */
	if(id < 3)
	{
		/* Refuse to setup timers that are already in use */
		tmu_t *t = timers[id].tmu;
		if(t->TCR.UNIE) return -1;

		/* Configure the registers of the target timer */
		t->TCOR = delay;
		t->TCNT = delay;
		t->TCR.TPSC = clock;

		/* Clear the interrupt flag */
		do t->TCR.UNF = 0;
		while(t->TCR.UNF);

		/* Enable interrupt and count on rising edge (SH7705) */
		t->TCR.UNIE = 1;
		t->TCR.CKEG = 0;
	}

	/* Extra timers have a simpler structure */
	else
	{
		etmu_t *t = timers[id].tmu;
		if(t->TCR.UNIE) return -1;

		/* Clear the interrupt flag */
		do t->TCR.UNF = 0;
		while(t->TCR.UNF);

		/* There is no clock input and no clock edge settings */
		t->TCOR = delay;

		/* TODO: FXCG50: does not always work on first try */
		do t->TCNT = delay;
		while(t->TCNT != delay);

		t->TCR.UNIE = 1;
	}

	/* Register the callback and its argument */
	if(timers[id].data)
	{
		timers[id].data->cb = callback;
		timers[id].data->arg = arg;
	}

	/* Return the timer id, since configuration was successful */
	return id;
}

/* timer_delay() - compute a delay constant from a duration in seconds */
uint32_t timer_delay(int tid, uint64_t delay_us)
{
	/* TODO: Proper timer_delay() */
	const clock_frequency_t *clock = clock_freq();
	uint64_t freq = clock->Pphi_f >> 2;

	/* fxcg50: Calculated = 29491200 but it's too low */
	/* TODO: Account for down spread spectrum in the CPG */
	// uint64_t freq = 29020000 >> 2;

	/* Extra timers all run at 32768 Hz */
	if(tid >= 3) freq = 32768;

	uint64_t product = freq * delay_us;
	return product / 1000000;
}

/* timer_control() - start or stop a timer
   @id     Timer ID to configure
   @state  0 to start the timer, 1 to stop it (nothing else!) */
static void timer_control(int id, int state)
{
	/* For standard timers, use the MPU's TSTR register */
	if(id < 3) *TSTR = (*TSTR | (1 << id)) ^ (state << id);
	/* Extra timers all have their own TSTR register */
	else ((etmu_t *)timers[id].tmu)->TSTR = state ^ 1;
}

/* timer_start() - start a configured timer */
void timer_start(int id)
{
	timer_control(id, 0);
}

/* timer_reload() - change a timer's delay constant for next interrupts */
void timer_reload(int id, uint32_t delay)
{
	if(id < 3) ((tmu_t *)timers[id].tmu)->TCOR = delay;
	else ((etmu_t *)timers[id].tmu)->TCOR = delay;
}

/* timer_pause() - stop a running timer */
void timer_pause(int id)
{
	timer_control(id, 1);
}

/* timer_stp() - stop and free a timer */
void timer_stop(int id)
{
	/* Stop the timer and disable UNIE to indicate that it's free */
	timer_pause(id);

	if(id < 3)
	{
		tmu_t *t = timers[id].tmu;
		t->TCR.UNIE = 0;

		/* Clear TCOR and TCNT */
		t->TCOR = 0xffffffff;
		t->TCNT = 0xffffffff;
	}
	else
	{
		etmu_t *t = timers[id].tmu;
		t->TCR.UNIE = 0;

		/* Also clear TCOR and TCNT to avoid spurious interrupts */
		t->TCOR = 0xffffffff;

		/* TODO: FXCG50: Again */
		do t->TCNT = 0xffffffff;
		while(t->TCNT + 1);

		do t->TCR.UNF = 0;
		while(t->TCR.UNF);
	}
}

//---
//	Predefined timer callbacks
//---

/* timer_timeout() - callback that sets a flag and halts the timer */
int timer_timeout(volatile void *arg)
{
	volatile int *x = arg;
	(*x)++;

	/* Always stop after firing once */
	return 1;
}

//---
//	Low-level functions
//---

/* timer_address() - get the address of a timer structure */
void *timer_address(int timer, volatile uint8_t **TSTR_arg)
{
	if((uint)timer < 2 && TSTR_arg) *TSTR_arg = TSTR;
	return (uint)timer < timer_count() ? timers[timer].tmu : NULL;
}

/* timer_clear() - clear an ETMU flag and possibly stop the timer
   @timer  Timer ID, must be an ETMU
   @stop   Non-zero to stop the timer */
void timer_clear(int timer, int stop)
{
	etmu_t *t = timers[timer].tmu;
	do t->TCR.UNF = 0;
	while(t->TCR.UNF);

	if(stop) timer_stop(timer);
}

//---
//	Driver initialization
//---

/* Interrupt handlers for standard timers (4 gates) */
extern void inth_tmu(void);
/* Interrupt handlers for extra timers */
extern void inth_etmu2(void);
extern void inth_etmu_help(void);
extern void inth_etmux(void);

#ifdef FX9860G
static void driver_sh3(void)
{
	timers[0].tmu = (void *)0xfffffe94;
	timers[1].tmu = (void *)0xfffffea0;
	timers[2].tmu = (void *)0xfffffeac;
	/* We must not change the event code of ETMU0 since it's translated to
	   its SH4 counterpart by the interrupt handler */
	timers[3].tmu = (void *)0xa44c0030;

	TSTR = (void *)0xfffffe92;
}
#endif /* FX9860G */

static void init(void)
{
	/* Install the standard TMU's interrupt handlers */
	void *h = gint_inthandler(0x400, inth_tmu, 128);

	/* User information in interrupt handlers */
	timers[0].data = h + 84;
	timers[1].data = h + 104;
	timers[2].data = h + 116;

	/* Stop all timers */
	*TSTR = 0;

	/* This driver uses the UNIE (UNderflow Interrupt Enable) bit of the
	   TCR register to indicate which timers are being used; reset them */
	for(int i = 0; i < 3; i++)
	{
		tmu_t *t = timers[i].tmu;

		t->TCOR = 0xffffffff;
		t->TCNT = 0xffffffff;

		do t->TCR.word = 0;
		while(t->TCR.word);

		/* Standard timers: TCR is provided to the interrupt handler */
		timers[i].data->TCR = &t->TCR;
	}

	/* Clear the extra timers */
	for(int i = 3; i < timer_count(); i++)
	{
		etmu_t *t = timers[i].tmu;

		/* Extra timers seem to generate interrupts as long as TCNT=0,
		   regardless of TSTR. I'm not entirely sure about this weird
		   behaviour, but for safety I'll set TCOR/TCNT to non-zero.
		   This may be related to difficulties when setting TCNT. */

		t->TSTR = 0;
		t->TCOR = 0xffffffff;

		/* TODO: FXCG50: Safety */
		do t->TCNT = 0xffffffff;
		while(t->TCNT + 1);

		/* Clear interrupts */
		do t->TCR.byte = 0;
		while(t->TCR.byte);
	}

	/* Install the extra timers. We need three extra timers for the
	   interrupt handlers to work, so install 3 on SH3, even if only one
	   actually exists */
	int limit = isSH3() ? 6 : 9;

	for(int i = 3; i < limit; i++)
	{
		void *handler = (i == 5) ? inth_etmu2 : inth_etmux;
		void *h = gint_inthandler(timers[i].event, handler, 32);

		timers[i].data = h + 24;

		if(i == 5) continue;
		uint32_t *data_id = (h + 20);
		*data_id = i;
	}

	/* Also install the helper handler */
	gint_inthandler(0xc60, inth_etmu_help, 32);

	/* Enable TMU0 at level 13, TMU1 at level 11, TMU2 at level 9 */
	gint_intlevel(0, 13);
	gint_intlevel(1, 11);
	gint_intlevel(2, 9);

	/* Enable the extra TMUs at level 7 */
	if(isSH3())
	{
		gint_intlevel(23, 7);
	}
	else
	{
		/* Unmask the standard timers' interrupts */
		SH7305_INTC.MSKCLR->IMR4 = 0x70;

		gint_intlevel(36, 7);
		gint_intlevel(25, 7);
		gint_intlevel(26, 7);
		gint_intlevel(18, 7);
		gint_intlevel(32, 7);
		gint_intlevel(44, 7);

		/* Unmask the extra timers' interrupts */
		SH7305_INTC.MSKCLR->IMR2 = 0x01;
		SH7305_INTC.MSKCLR->IMR5 = 0x06;
		SH7305_INTC.MSKCLR->IMR6 = 0x18;
		SH7305_INTC.MSKCLR->IMR8 = 0x02;
	}

	/* Record details in gint's hardware information interface */

	gint[HWTMU]  = HW_LOADED;
	gint[HWETMU] = HW_LOADED | (isSH3() ? HWETMU_1 : HWETMU_6);

	for(int i = 3; i < timer_count(); i++)
	{
		etmu_t *t = timers[i].tmu;
		int v = !(t->TCOR + 1)
		     && !(t->TCNT + 1)
		     && !(t->TSTR)
		     && !(t->TCR.UNF)
		     && !(t->TCR.UNIE);
		gint[HWETMU] |= v << (i - 1);
	}
}

//---
//	Status function
//---

#ifdef GINT_BOOT_LOG

/* tmu_status() - status string of extra TMUs for the boot log
   The status string has a two-character code for each of the extra timers.

   The first character is a digit character describing a value between 0 and 7.
   * Bit 0 is set if TCOR=0xffffffff
   * Bit 1 is set if TCNT=0xffffffff
   * Bit 2 is set if TSTR=0

   The second character indicates the status of interrupts.
   * "D" (Disabled) if UNIE=0, UNF=0
   * "L" (Low)      if UNIE=1, UNF=0
   * "H" (High)     if UNIE=1, UNF=1
   * "!" (Error)    if UNIE=0, UNF=1

   So the normal status string would be made of "7D"'s. */
static const char *tmu_status(void)
{
	static char status[18] = "ETMU ";

	int j = 5;
	for(int i = 3; i < timer_count(); i++)
	{
		etmu_t *t = timers[i].tmu;
		int v1 = (!(t->TCOR + 1))
		       | (!(t->TCNT + 1) << 1)
		       | (!(t->TSTR) << 2);
		int v2 = (t->TCR.UNF << 1) | (t->TCR.UNIE);

		status[j++] = '0' + v1;
		status[j++] = "DLH!"[v2];
	}
	status[j] = 0;

	return status;
}

#endif /* GINT_BOOT_LOG */

//---
//	Context system for this driver
//---

typedef struct
{
	tmu_t std[3];
	etmu_t extra[6];
	uint8_t TSTR;

} GPACKED(4) ctx_t;

/* Allocate a system buffer in gint's BSS area */
GBSS static ctx_t sys_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;

	for(int i = 0; i < 3; i++)
	{
		tmu_t *t = timers[i].tmu;
		ctx->std[i].TCOR	= t->TCOR;
		ctx->std[i].TCNT	= t->TCNT;
		ctx->std[i].TCR.word	= t->TCR.word;
	}

	ctx->TSTR = *TSTR;

	for(int i = 0; i < timer_count() - 3; i++)
	{
		etmu_t *t = timers[i + 3].tmu;
		ctx->extra[i].TCOR	= t->TCOR;
		ctx->extra[i].TCNT	= t->TCNT;
		ctx->extra[i].TCR.byte	= t->TCR.byte;
		ctx->extra[i].TSTR	= t->TSTR;
	}
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;

	for(int i = 0; i < 3; i++)
	{
		tmu_t *t = timers[i].tmu;
		t->TCNT		= ctx->std[i].TCNT;
		t->TCOR		= ctx->std[i].TCOR;
		t->TCR.word	= ctx->std[i].TCR.word;
	}

	*TSTR = ctx->TSTR;

	for(int i = 0; i < timer_count() - 3; i++)
	{
		etmu_t *t = timers[i + 3].tmu;

		/* This thing is being unloaded while interrupts are disabled,
		   so we don't have to heed for ctx->extra[i].TCNT = 0, which
		   can generate interrupts. I tried to do t->TCNT = 0xffffffff
		   then restore ctx->extra[i], but it causes hangs on SH3 when
		   the overclock level is too high. */
		t->TCNT		= ctx->extra[i].TCNT;
		t->TCOR		= ctx->extra[i].TCOR;
		t->TCR.byte	= ctx->extra[i].TCR.byte;
		t->TSTR		= ctx->extra[i].TSTR;
	}
}

//---
//	Driver structure definition
//---

gint_driver_t drv_tmu = {
	.name		= "TMU",
	.driver_sh3	= GINT_DRIVER_SH3(driver_sh3),
	.init		= init,
	.status		= GINT_DRIVER_STATUS(tmu_status),
	.ctx_size	= sizeof(ctx_t),
	.sys_ctx	= &sys_ctx,
	.ctx_save	= ctx_save,
	.ctx_restore	= ctx_restore,
};

GINT_DECLARE_DRIVER(2, drv_tmu);
