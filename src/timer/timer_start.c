#include <internals/timer.h>
#include <timer.h>

/*
	timer_start()
	Configures and starts a timer.
*/
void timer_start(int timer, int delay, int prescaler, void (*callback)(void),
	int repeats)
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
	timers[timer].callback = callback;
	timers[timer].repeats = repeats;

	// Starting the timer.
	*tstr |= byte;
}
