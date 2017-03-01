#include <timer.h>
#include <internals/timer.h>
#include <modules/timer.h>

//---
//	Public API
//---

timer_t htimers[3] = { 0 };

/*
	htimer_setup()
	Configures a hardware timer.
*/
timer_t *htimer_setup(timer_hard_t id, uint32_t constant, timer_input_t input,
	int repeats)
{
	if(id < 0 || id >= 3 || htimers[id].used) return NULL;
	timer_t *timer = &htimers[id];

	// We don't care about this.
	timer->ms_delay		= 0;
	timer->ms_elapsed	= 0;

	timer->repeats_left	= repeats;

	timer->used		= 1;
	timer->active		= 0;
	timer->virtual		= 0;
	timer->vsupport		= 0;
	timer->events		= 0;

	timer->callback		= NULL;
	timer->argument		= NULL;

	// Time to set up the real thing.
	volatile mod_tmu_timer_t *tmu = TMU.timers[id];

	tmu->TCOR		= constant;
	tmu->TCNT		= constant;
	tmu->TCR.TPSC		= input;

	tmu->TCR.UNF		= 0;	// Clear the interrupt flag.
	tmu->TCR.UNIE		= 1;	// Enable underflow interrupt.
	tmu->TCR.CKEG		= 0;	// Count on rising edge (SH7705).

	return timer;
}

/*
	htimer_reload()
	Reloads a hardware timer without starting or stopping it.
*/
void htimer_reload(timer_hard_t id, uint32_t new_constant)
{
	volatile mod_tmu_timer_t *tmu = TMU.timers[id];

	// This is not much but we want to make the transition as swift as
	// possible and I can prove that at least one cycle will be lost in
	// processor operation. Thus...
	tmu->TCNT = new_constant - 1;
	tmu->TCOR = new_constant;
}
