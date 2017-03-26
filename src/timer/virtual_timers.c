#include <timer.h>
#include <clock.h>
#include <internals/timer.h>
#include <modules/timer.h>

//---
//	Public API
//---

timer_t vtimers[TIMER_SLOTS] = { 0 };

/*
	timer_create()
	Creates a virtual timer and configures its delay and repetition count.
*/
timer_t *timer_create(int ms_delay, int repeats)
{
	if(ms_delay <= 0) return NULL;
	timer_t *timer = vtimers;

	// Finding an available virtual slot.
	while(timer < vtimers + TIMER_SLOTS && timer->used) timer++;
	if(timer >= vtimers + TIMER_SLOTS) return NULL;

	timer->ms_delay		= ms_delay;
	timer->ms_elapsed	= 0;
	timer->repeats_left	= repeats;

	timer->used		= 1;
	timer->active		= 0;
	timer->virtual		= 1;
	timer->vsupport		= 0;
	timer->events		= 0;

	timer->callback		= NULL;
	timer->argument		= NULL;

	return timer;
}

/*
	timer_reload()
	Changes a virtual timer's delay. The timer is not stopped nor started:
	it keeps running or waiting. Events that were waiting to be handled are
	dropped and the number of repeats left is not changed. The timer
	restarts counting from 0 regardless of how much time had elapsed since
	it last fired.
*/
void timer_reload(timer_t *timer, int new_ms_delay)
{
	if(!timer->virtual) return;

	timer->ms_delay		= new_ms_delay;
	timer->ms_elapsed	= 0;

	vtimer_updateAll();
}

/*
	timer_destroy()
	Destroys a virtual timer. This virtual timer pointer becomes invalid
	and should not be used anymore.
*/
void timer_destroy(timer_t *timer)
{
	timer->events	= 0;
	timer->active	= 0;
	timer->used	= 0;

	vtimer_updateAll();
}



//---
//	Virtual timers management
//---

static int current_delay = 0;
static uint32_t current_constant = 0;

/*
	vtimer_interrupt()
	Interrupt handling subsystem for the virtual timers.
*/
void vtimer_interrupt(void)
{
	// Do we need to recompute the hardware support frequency?
	int recalc = 0;

	// No timer is running.
	if(!current_delay) return;

	// Update them all and call the required callbacks. Stop the ones that
	// have been running for long enough.
	for(timer_t *timer = vtimers; timer < vtimers + TIMER_SLOTS; timer++)
	if(timer->used)
	{
		timer->ms_elapsed += current_delay;

		// This should happen only once but in case there is a problem,
		// this loop will ensure that at least the correct amount of
		// callbacks is executed.
		// We could divide but it would be slower.
		while(timer->ms_elapsed >= timer->ms_delay)
		{
			// We don't call the callbacks now because we may need
			// to update the timer frequency later and we want to
			// get there asap.
			timer->events++;
			timer->ms_elapsed -= timer->ms_delay;
		}

		// We would need to stop one virtual timer.
		if(timer->repeats_left > 0 && timer->repeats_left <=
			timer->events) recalc = 1;
	}

	if(recalc) vtimer_updateAll();

	for(timer_t *timer = vtimers; timer < vtimers + TIMER_SLOTS; timer++)
	if(timer->used)
	{
		while(timer->active && timer->events)
		{
			timer_callback_event(timer);
			timer->events--;
		}
		timer->used = timer->active;
	}
}

/*
	vtimer_update()
	Swiftly updates the hardware support counter and constant to keep up
	with new kinds of timers being added while disturbing the counting flow
	as little as possible.
*/
static void vtimer_update(int new_delay)
{
	volatile mod_tmu_timer_t *tmu = TMU.timers[timer_virtual];
	timer_t *timer = &htimers[timer_virtual];

	if(new_delay == current_delay) return;

	if(!new_delay)
	{
		current_delay		= 0;
		current_constant	= 0;

		// At this point the virtual support timer was running and has
		// been stopped, thus the hardware timer structure is filled
		// properly so we can use this function.
		timer_stop(timer);
		return;
	}
	uint32_t new_constant = clock_setting(new_delay, clock_ms);

	// The transition needs to be as smooth as possible. We have probably
	// spent a lot of time calculating this new GCD delay so we want to
	// take into consideration the current value of the timer counter.
	// The -1 is here to take into account the time between reading and
	// writing the new constant (~10 I_phi cycles thus usually 5 P_phi
	// cycles; given this timer runs on P_phi / 4 this makes 1 unit).
	// Here we suppose that the new TCNT is positive, i.e that the update
	// happened in less than 1 ms. This is reasonable as long as this
	// happens *before* calling callbacks.
	tmu->TCNT = new_constant - (current_constant - tmu->TCNT - 1);
	tmu->TCOR = new_constant;

	if(!current_delay)
	{
		// Ups, we've been using a random counter; the timer's not
		// running.
		tmu->TCNT		= tmu->TCOR;
		// Set it up then.
		tmu->TCR.TPSC		= timer_Po_4;
		tmu->TCR.UNF		= 0;
		tmu->TCR.UNIE		= 1;
		tmu->TCR.CKEG		= 0;
		// Tell them that the virtual timer support is running there.
		timer->used		= 1;
		timer->active		= 0;
		timer->virtual		= 0;
		timer->vsupport		= 1;
		timer->events		= 0;
		// And let's roll!
		timer_start(timer);
	}

	current_delay = new_delay;
	current_constant = new_constant;
}

/*
	gcd()
	Well, the Euclidean algorithm. That's O(ln(max(a, b))) FWIW.
*/
static int gcd(int a, int b)
{
	while(b)
	{
		int r = a % b;
		a = b;
		b = r;
	}

	return a;
}

/*
	vtimer_updateOne()
	Update the virtual timer hardware support timer, knowing that a virtual
	timer with the given delay has been started.
*/
void vtimer_updateOne(int additional_delay)
{
	vtimer_update(gcd(current_delay, additional_delay));
}

/*
	vtimer_updateAll()
	Updates the virtual timer hardware support after computing the GCD of
	all virtual timers delays. This computation is rather long (especially
	this modulo can afaik only be achieved using division on SuperH and
	it's a 70-cycle operation) so it should be avoided when possible.
*/

void vtimer_updateAll(void)
{
	int gcd_delay = 0;

	for(timer_t *timer = vtimers; timer < vtimers + TIMER_SLOTS; timer++)
	if(timer->used && timer->active && (timer->repeats_left <= 0 ||
		timer->repeats_left > timer->events))
	{
		gcd_delay = gcd(gcd_delay, timer->ms_delay);
	}

	vtimer_update(gcd_delay);
}
