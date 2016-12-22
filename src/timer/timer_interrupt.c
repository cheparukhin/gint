#include <internals/timer.h>
#include <timer.h>

#include <stddef.h>

struct Timer timers[3] = { { NULL, 0 }, { NULL, 0 }, { NULL, 0 } };

/*
	timer_interrupt()
	Handles the interrupt for the given timer.
*/
void timer_interrupt(int timer)
{
	volatile struct mod_tmu *tmu;
	timer_get(timer, &tmu, NULL);

	tmu->TCR.UNF = 0;
	if(timers[timer].callback) timers[timer].callback();

	// Reducing the number of repetitions left, if not infinite.
	if(!timers[timer].repeats) return;
	if(timers[timer].repeats == 1) timer_stop(timer);
	else timers[timer].repeats--;
}
