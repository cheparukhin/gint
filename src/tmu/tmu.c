//---
//	gint:tmu - Timer operation
//---

#include <gint/timer.h>
#include <gint/drivers.h>
#include <gint/gint.h>
#include <gint/clock.h>
#include <gint/intc.h>
#include <gint/mpu/tmu.h>

#include <stdarg.h>

#undef timer_setup

//---
//	Driver storage
//---

/* inth_data_t - data storage inside interrupt handlers */
typedef struct
{
	void *function;		/* User-provided callback */
	uint32_t arg;		/* Argument for function */
	volatile void *TCR;	/* TCR address for TMU */

} GPACKED(4) inth_data_t;

/* This array references the storage areas of all timer handlers */
static inth_data_t *timers[9] = { NULL };

/* Arrays of standard and extra timers */
static tmu_t *TMU = SH7305_TMU.TMU;
static etmu_t *ETMU = SH7305_ETMU;
/* TSTR register for standard timers */
static volatile uint8_t *TSTR = &SH7305_TMU.TSTR;

//---
//	Local functions
//---

/* configure(): Configure a fixed timer */
static void configure(int id, uint32_t delay, int clock, void *f, uint32_t arg)
{
	if(id < 3)
	{
		/* Refuse to setup timers that are already in use */
		tmu_t *T = &TMU[id];
		if(T->TCR.UNIE || *TSTR & (1 << id)) return;

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
		if(T->TCR.UNIE) return;

		/* No clock input and clock edge here. But TCR and TCNT need
		   some time to execute the write */
		do T->TCR.UNF = 0;
		while(T->TCR.UNF);

		do T->TCOR = delay;
		while(T->TCOR != delay);

		do T->TCNT = delay;
		while(T->TCNT != delay);

		T->TCR.UNIE = 1;
	}

	timers[id]->function = f;
	timers[id]->arg = arg;
}

/* matches(): Check if a timer matches the provided specification and delay */
static int matches(int id, int spec, uint32_t delay)
{
	/* A specific idea only matches the exact timer */
	if(spec >= 0) return id == (spec & 0xf);
	/* TIMER_ANY always matches ETMU only for delays at least 100 µs */
	if(spec == TIMER_ANY) return (id < 3 || delay >= 100);
	/* TIMER_TMU and TIMER_ETMU match as you'd expect */
	if(spec == TIMER_TMU) return (id < 3);
	if(spec == TIMER_ETMU) return (id >= 3);
	/* Default is not matching */
	return 0;
}

/* available(): Check if a timer is available (UNIE cleared, not running) */
static int available(int id)
{
	/* The timer should also be installed... */
	if(!timers[id]) return 0;

	if(id < 3)
	{
		tmu_t *T = &TMU[id];
		return !T->TCR.UNIE && !(*TSTR & (1 << id));
	}
	else
	{
		etmu_t *T = &ETMU[id-3];
		return !T->TCR.UNIE && !T->TSTR;
	}
}

/* stop_callback(): Empty callback that stops the timer */
static int stop_callback(void)
{
	return TIMER_STOP;
}

//---
//	Timer API
//---

/* timer_setup(): Reserve and configure a timer */
int timer_setup(int spec, uint64_t delay, timer_callback_t function, ...)
{
	int clock = 0;

	/* Get the optional argument */
	va_list va;
	va_start(va, function);
	uint32_t arg = va_arg(va, uint32_t);
	va_end(va);

	/* Default value for the callback */
	if(!function.v) function.v = stop_callback;

	/* Find a matching timer, starting from the slowest timers with the
	   smallest interrupt priorities all the way up to TMU0 */
	for(int id = timer_count() - 1; id >= 0; id--)
	{
		if(!matches(id, spec, delay) || !available(id)) continue;

		/* If ID is a TMU, choose a timer prescaler. Assuming the worst
		   running Pphi of ~48 MHz, select the finest resolution that
		   allows the requested delay to be represented. */
		if(id < 3 && spec >= 0)
		{
			/* Explicit timers with clock in the specification */
			clock = (spec >> 4) & 0xf;
		}
		else if(id < 3)
		{
			uint64_t sec = 1000000;

			/* Pphi/4 until 350 seconds */
			if(delay <= 350 * sec) clock = TIMER_Pphi_4;
			/* Pphi/16 until 1430 seconds */
			else if(delay <= 1430 * sec) clock = TIMER_Pphi_16;
			/* Pphi/64 until 5720 seconds */
			else if(delay <= 5720 * sec) clock = TIMER_Pphi_64;
			/* Pphi/256 otherwise */
			else clock = TIMER_Pphi_256;
		}

		/* Find the delay constant for that timer and clock */
		if(spec < 0) delay = timer_delay(id, delay, clock);

		configure(id, delay, clock, function.v, arg);
		return id;
	}

	return -1;
}

/* timer_delay() - compute a delay constant from a duration in seconds */
uint32_t timer_delay(int id, uint64_t delay_us, int clock)
{
	uint64_t freq;

	if(id < 3)
	{
		const clock_frequency_t *cpg = clock_freq();
		freq = cpg->Pphi_f;
		if(clock == TIMER_Pphi_4)   freq >>= 2;
		if(clock == TIMER_Pphi_16)  freq >>= 4;
		if(clock == TIMER_Pphi_64)  freq >>= 6;
		if(clock == TIMER_Pphi_256) freq >>= 8;
	}
	else
	{
		/* ETMU all run on TCLK at 32768 Hz */
		freq = 32768;
	}

	/* fxcg50: Calculated = 29491200 but it's too low */
	/* TODO: Account for down spread spectrum in the CPG */
	// uint64_t freq = 29020000 >> 2;

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
int timer_timeout(void volatile *arg)
{
	int volatile *x = arg;
	if(x) (*x)++;
	return TIMER_STOP;
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
		do TMU[id].TCR.word = 0;
		while(TMU[id].TCR.word);

		TMU[id].TCOR = 0xffffffff;
		TMU[id].TCNT = 0xffffffff;

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

		/* On SH3, the ETMU handler is not at an offset of 0x840 (event
		   code 0xc40) but at an offset of 0xc0 */
		uint32_t *etmu_offset = h + 16;
		if(isSH3()) *etmu_offset = *etmu_offset - 0xe80 + 0x2c0;

		uint32_t *data_id = h + 20;
		*data_id = i;
	}

	/* Also install the helper handler */
	gint_inthandler(0xc60, inth_etmu_help, 32);

	/* Enable TMU0 at level 13, TMU1 at level 11, TMU2 at level 9 */
	intc_priority(INTC_TMU_TUNI0, 13);
	intc_priority(INTC_TMU_TUNI1, 11);
	intc_priority(INTC_TMU_TUNI2, 9);

	/* Enable the extra TMUs at level 7 */
	intc_priority(INTC_ETMU_TUNI0, 7);
	if(isSH4())
	{
		intc_priority(INTC_ETMU_TUNI1, 7);
		intc_priority(INTC_ETMU_TUNI2, 7);
		intc_priority(INTC_ETMU_TUNI3, 7);
		intc_priority(INTC_ETMU_TUNI4, 7);
		intc_priority(INTC_ETMU_TUNI5, 7);
	}

	/* Record details in gint's hardware information interface */

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

struct stored_timer {
	uint32_t TCOR;
	uint32_t TCNT;
	uint16_t TCR;
	uint16_t TSTR;
};

typedef struct
{
	struct stored_timer t[9];
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
		struct stored_timer *c = &ctx->t[i];
		c->TCOR = TMU[i].TCOR;
		c->TCNT = TMU[i].TCNT;
		c->TCR  = TMU[i].TCR.word;
	}
	for(int i = 3; i < timer_count(); i++)
	{
		struct stored_timer *c = &ctx->t[i];
		etmu_t *T = &ETMU[i-3];

		/* Don't snapshot an interrupt state, because the timer state
		   is sometimes garbage protected by a masked interrupt. */
		c->TCOR = T->TCOR ? T->TCOR : 0xffffffff;
		c->TCNT = T->TCNT ? T->TCNT : c->TCOR;
		c->TCR  = T->TCR.byte & 0xd;
		c->TSTR	= T->TSTR;
	}
}

static void ctx_restore(void *buf)
{
	ctx_t *ctx = buf;
	*TSTR = 0;

	for(int i = 0; i < 3; i++)
	{
		struct stored_timer *c = &ctx->t[i];
		TMU[i].TCOR = c->TCOR;
		TMU[i].TCNT = c->TCNT;
		TMU[i].TCR.word = c->TCR;
	}
	for(int i = 3; i < timer_count(); i++)
	{
		struct stored_timer *c = &ctx->t[i];
		etmu_t *T = &ETMU[i-3];

		do T->TCOR = c->TCOR;
		while(T->TCOR != c->TCOR);

		T->TSTR = c->TSTR;

		/* Remember that TCNT and TCR take time to write to */
		do T->TCNT = c->TCNT;
		while(T->TCNT != c->TCNT);

		do T->TCR.byte = c->TCR;
		while(T->TCR.byte != c->TCR);
	}

	*TSTR = ctx->TSTR;
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
