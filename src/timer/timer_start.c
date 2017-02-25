#include <internals/timer.h>
#include <timer.h>

/*
	timer_start2()
	Configures and starts a time using a clock count and a prescaler.
*/
void timer_start2(int timer, int delay, int prescaler, void *callback,
	void *data, int repeats)
{
	volatile struct mod_tmu *tmu;
	volatile unsigned char *tstr;
	int byte = (1 << timer);

	timer_get(timer, &tmu, &tstr);

	// Loading the counter, the constant and the prescaler/
	tmu->TCOR = delay;
	tmu->TCNT = delay;
	tmu->TCR.TPSC = prescaler;

	// Resetting underflow flag and enabling interruptions.
	tmu->TCR.UNF = 0;
	tmu->TCR.UNIE = 1;

	// Counting on rising edge (ignored on SH7305).
	tmu->TCR.CKEG = 0;

	// Loading the structure information.
	timers[timer].callback	= callback;
	timers[timer].data	= data;
	timers[timer].repeats	= repeats;

	// Starting the timer.
	*tstr |= byte;
}

/*
	timer_start()
	Configures and starts a timer using a delay, or a frequency, and the
	associated unit.
*/
void timer_start(int timer, int delay, enum ClockUnit unit, void *callback,
	void *data, int repeats)
{
	timer_start2(timer, clock_setting(delay, unit), TIMER_Po_4, callback,
		data, repeats);
}
