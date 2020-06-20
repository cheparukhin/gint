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
//	Driver storage
//---

/* inth_data_t - data storage inside interrupt handlers */
typedef struct
{
	int (*cb)(volatile void *arg);	/* User-provided callback */
	volatile void *arg;		/* Argument for [callback] */
	volatile void *TCR;		/* TCR address for TMU */

} GPACKED(4) inth_data_t;

/* This array references the storage areas of all timer handlers */
GDATA static inth_data_t *timers[9] = { NULL };

/* Arrays of standard and extra timers */
GDATA static tmu_t *TMU = SH7305_TMU.TMU;
GDATA static etmu_t *ETMU = SH7305_ETMU;
/* TSTR register for standard timers */
GDATA static volatile uint8_t *TSTR = &SH7305_TMU.TSTR;

//---
//	Timer API
//---

/* timer_setup() - set up a timer */
int timer_setup(int id, uint32_t delay, timer_input_t clock,
	int (*callback)(volatile void *arg), volatile void *arg)
{
	/* Timer is not installed (TCR address is really required) */
	if(!timers[id]) return -1;

	if(id < 3)
	{
		/* Refuse to setup timers that are already in use */
		tmu_t *T = &TMU[id];
		if(T->TCR.UNIE || *TSTR & (1 << id)) return -1;

		/* Configure the counter, clear interrupt flag*/
		T->TCOR = delay;
		T->TCNT = delay;
		T->TCR.TPSC = clock;
		do T->TCR.UNF = 0;
		while(T->TCR.UNF);

		/* Enable interrupt and count on rising edge (SH7705) */
		T->TCR.UNIE = 1;
		T->TCR.CKEG = 0;
	}
	else
	{
		etmu_t *T = &ETMU[id-3];
		if(T->TCR.UNIE) return -1;

		/* No clock input and clock edge here. But TCR and TCNT need a
		   delay to retain the value */
		do T->TCR.UNF = 0;
		while(T->TCR.UNF);

		do T->TCOR = delay;
		while(T->TCOR != delay);

		do T->TCNT = delay;
		while(T->TCNT != delay);

		T->TCR.UNIE = 1;
	}

	timers[id]->cb = callback;
	timers[id]->arg = arg;
	return id;
}

/* timer_delay() - compute a delay constant from a duration in seconds */
uint32_t timer_delay(int id, uint64_t delay_us)
{
	/* TODO: Proper timer_delay() */
	const clock_frequency_t *clock = clock_freq();
	uint64_t freq = clock->Pphi_f >> 2;

	/* fxcg50: Calculated = 29491200 but it's too low */
	/* TODO: Account for down spread spectrum in the CPG */
	// uint64_t freq = 29020000 >> 2;

	/* Extra timers all run at 32768 Hz */
	if(id >= 3) freq = 32768;

	uint64_t product = freq * delay_us;
	return product / 1000000;
}

/* timer_control() - start or stop a timer
   @id     Timer ID to configure
   @state  0 to start the timer, 1 to stop it (nothing else!) */
static void timer_control(int id, int state)
{
	if(id < 3) *TSTR = (*TSTR | (1 << id)) ^ (state << id);
	else ETMU[id-3].TSTR = state ^ 1;
}

/* timer_start() - start a configured timer */
void timer_start(int id)
{
	timer_control(id, 0);
}

/* timer_reload() - change a timer's delay constant for next interrupts */
void timer_reload(int id, uint32_t delay)
{
	if(id < 3) TMU[id].TCOR = delay;
	else ETMU[id-3].TCOR = delay;
}

/* timer_pause() - stop a running timer */
void timer_pause(int id)
{
	timer_control(id, 1);
}

/* timer_stop() - stop and free a timer */
GMAPPED void timer_stop(int id)
{
	/* Stop the timer and disable UNIE to indicate that it's free */
	timer_pause(id);

	if(id < 3)
	{
		TMU[id].TCR.UNIE = 0;
		TMU[id].TCOR = 0xffffffff;
		TMU[id].TCNT = 0xffffffff;
	}
	else
	{
		etmu_t *T = &ETMU[id-3];
		T->TCR.UNIE = 0;

		/* Also clear TCOR and TCNT to avoid spurious interrupts */
		do T->TCOR = 0xffffffff;
		while(T->TCOR + 1);

		do T->TCNT = 0xffffffff;
		while(T->TCNT + 1);

		do T->TCR.UNF = 0;
		while(T->TCR.UNF);
	}
}

/* timer_wait() - wait until a timer is stopped */
void timer_wait(int id)
{
	if(id < 3)
	{
		tmu_t *T = &TMU[id];
		/* Sleep if an interruption will wake us up */
		while(*TSTR & (1 << id)) if(T->TCR.UNIE) sleep();
	}
	else
	{
		etmu_t *T = &ETMU[id-3];
		while(T->TSTR) if(T->TCR.UNIE) sleep();
	}
}

//---
//	Predefined timer callbacks
//---

/* timer_timeout() - callback that sets a flag and halts the timer */
GMAPPED int timer_timeout(volatile void *arg)
{
	volatile int *x = arg;
	(*x)++;
	return 1;
}

//---
//	Low-level functions
//---

/* timer_clear() - clear an ETMU flag
   @timer  Timer ID, must be an ETMU */
GMAPPED void timer_clear(int id)
{
	do ETMU[id-3].TCR.UNF = 0;
	while(ETMU[id-3].TCR.UNF);
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
	TMU = SH7705_TMU.TMU;
	ETMU = SH7705_ETMU;
	TSTR = &SH7705_TMU.TSTR;
}
#endif /* FX9860G */

static void init(void)
{
	uint16_t etmu_event[6] = { 0x9e0, 0xc20, 0xc40, 0x900, 0xd00, 0xfa0 };
	*TSTR = 0;

	/* Install the standard TMU's interrupt handlers */
	void *h = gint_inthandler(0x400, inth_tmu, 128);
	timers[0] = h + 84;
	timers[1] = h + 104;
	timers[2] = h + 116;

	/* Clear every timer to avoid surprises */
	for(int id = 0; id < 3; id++)
	{
		TMU[id].TCOR = 0xffffffff;
		TMU[id].TCNT = 0xffffffff;

		do TMU[id].TCR.word = 0;
		while(TMU[id].TCR.word);

		/* Standard timers: TCR is provided to the interrupt handler */
		timers[id]->TCR = &TMU[id].TCR;
	}
	for(int id = 0; id < timer_count()-3; id++)
	{
		etmu_t *T = &ETMU[id];

		/* Extra timers seem to generate interrupts as long as TCNT=0,
		   regardless of TSTR. I'm not entirely sure about this weird
		   behaviour, but for safety I'll set TCOR/TCNT to non-zero. */
		T->TSTR = 0;
		do T->TCOR = 0xffffffff;
		while(T->TCOR + 1);

		/* Also TCNT and TCR take some time to record changes */
		do T->TCNT = 0xffffffff;
		while(T->TCNT + 1);
		do T->TCR.byte = 0;
		while(T->TCR.byte);
	}

	/* Install the extra timers. The interrupt handler takes 3 gates, so we
	   install 3 even on SH3 where there's only one */
	int limit = isSH3() ? 6 : 9;

	for(int i = 3; i < limit; i++)
	{
		void *handler = (i == 5) ? inth_etmu2 : inth_etmux;
		void *h = gint_inthandler(etmu_event[i-3], handler, 32);

		timers[i] = h + 24;

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
	if(isSH3()) gint_intlevel(23, 7);
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

	for(int i = 0; i < timer_count() - 3; i++)
	{
		etmu_t *T = &ETMU[i];
		int v = !(T->TCOR + 1)
		     && !(T->TCNT + 1)
		     && !(T->TSTR)
		     && !(T->TCR.UNF)
		     && !(T->TCR.UNIE);
		gint[HWETMU] |= v << (i + 2);
	}
}

//---
//	Context system for this driver
//---

typedef struct
{
	tmu_t std[3];
	etmu_t extra[6];
	uint8_t TSTR;
} ctx_t;

/* Allocate a system buffer in gint's BSS area */
GBSS static ctx_t sys_ctx, gint_ctx;

static void ctx_save(void *buf)
{
	ctx_t *ctx = buf;
	ctx->TSTR = *TSTR;

	for(int i = 0; i < 3; i++)
	{
		tmu_t *c = &ctx->std[i];
		c->TCOR = TMU[i].TCOR;
		c->TCNT = TMU[i].TCNT;
		c->TCR.word = TMU[i].TCR.word;
	}
	for(int i = 0; i < timer_count() - 3; i++)
	{
		etmu_t *T = &ETMU[i], *c = &ctx->extra[i];

		/* Don't snapshot an interrupt state, because the timer state
		   is sometimes garbage protected by a masked interrupt. */
		c->TCOR = T->TCOR ? T->TCOR : 0xffffffff;
		c->TCNT = T->TCNT ? T->TCNT : c->TCOR;
		c->TCR.byte = T->TCR.byte & 0xd;
		c->TSTR	= T->TSTR;
	}
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;
	*TSTR = ctx->TSTR;

	for(int i = 0; i < 3; i++)
	{
		tmu_t *c = &ctx->std[i];
		TMU[i].TCNT = c->TCNT;
		TMU[i].TCOR = c->TCOR;
		TMU[i].TCR.word = c->TCR.word;
	}
	for(int i = 0; i < timer_count() - 3; i++)
	{
		etmu_t *T = &ETMU[i], *c = &ctx->extra[i];

		do T->TCOR = c->TCOR;
		while(T->TCOR != c->TCOR);

		T->TSTR = c->TSTR;

		/* Remember that TCNT and TCR take time to write to */
		do T->TCNT = c->TCNT;
		while(T->TCNT != c->TCNT);

		do T->TCR.byte = c->TCR.byte;
		while(T->TCR.byte != c->TCR.byte);
	}
}

//---
//	Driver structure definition
//---

gint_driver_t drv_tmu = {
	.name         = "TMU",
	.driver_sh3   = GINT_DRIVER_SH3(driver_sh3),
	.init         = init,
	.sys_ctx      = &sys_ctx,
	.gint_ctx     = &gint_ctx,
	.ctx_save     = ctx_save,
	.ctx_restore  = ctx_restore,
};

GINT_DECLARE_DRIVER(2, drv_tmu);
