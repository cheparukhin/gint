#include <timer.h>
#include <internals/timer.h>
#include <modules/timer.h>
#include <events.h>

/*
	timer_attach()
	Attaches a callback to a virtual or hardware timer.
*/
void timer_attach(timer_t *timer, void *callback, void *argument)
{
	if(!timer) return;

	timer->callback = callback;
	timer->argument = argument;
}

/*
	timer_start()
	Starts a virtual or hardware timer. If the timer has a callback
	attached, then the callback function will start being called regularly;
	otherwise, the timer will start pushing ET_Timer events to the event
	queue. It is advised, not to change a timer's configuration while it's
	running.
*/
void timer_start(timer_t *timer)
{
	if(!timer) return;
	timer->active = 1;

	if(timer->virtual) vtimer_updateOne(timer->ms_delay);
	else TMU.TSTR->byte |= (1 << (timer - htimers));
}

/*
	timer_stop()
	Pauses a virtual or hardware timer. The timer stops counting and can be
	started again later.
*/
void timer_stop(timer_t *timer)
{
	if(!timer) return;
	timer->active = 0;

	if(timer->virtual) vtimer_updateAll();
	else
	{
		TMU.TSTR->byte &= ~(1 << (timer - htimers));
		timer->used = 0;
	}
}

/*
	timer_interrupt()
	Handles the interrupt for the given timer channel.
*/
void timer_interrupt(int channel)
{
	// Is this the virtual timer support?
	if(htimers[channel].vsupport) vtimer_interrupt();
	else timer_callback_event(&htimers[channel]);

	// Clearing the interrupt flag.
	TMU.timers[channel]->TCR.UNF = 0;
}

/*
	timer_callback_event()
	Executes the callback of a timer, or pushes a new timer event depending
	on the timer configuration. Also reduces the amount of repeats left and
	clears the active flag (or stops the hardware timer) if this number
	falls from one to zero.
*/
void timer_callback_event(timer_t *timer)
{
	if(!timer) return;

	// Callback-type timers.
	if(timer->callback)
	{
		if(!timer->argument)
		{
			void (*fun)(void) = timer->callback;
			fun();
		}
		else
		{
			void (*fun)(void *arg) = timer->callback;
			fun(timer->argument);
		}
	}

	// Event-type timers.
	else
	{
		event_t event = {
			.type	= event_timer_underflow,
			.timer	= timer
		};
		event_push(event);
	}

	// Reducing the number of repeats and stopping the timer if required.
	if(timer->repeats_left > 0)
	{
		if(!--timer->repeats_left)
		{
			if(timer->virtual) timer->active = 0;
			else timer_stop(timer);
		}
	}
}
