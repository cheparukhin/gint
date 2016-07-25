#include <internals/timer.h>
#include <timer.h>

/*
	timer_start()
	Configures and starts a timer.
*/
void timer_start(int timer, int delay, int prescaler, void (*callback)(void),
	int repeats)
{
	// Getting the timer address. Using a byte to alter TSTR.
	struct mod_tmu *tmu;
	unsigned char *tstr;
	int byte = (1 << timer);
	timer_get(timer, &tmu, &tstr);

	// Setting the constant register.
	(*tmu).TCOR = delay;
	// Loading the delay in the counter.
	(*tmu).TCNT = delay;

	// Resetting underflow flag.
	(*tmu).TCR.UNF = 0;
	// Enabling interruptions on underflow.
	(*tmu).TCR.UNIE = 1;
	// Counting on rising edge. On SH7305 these two bits are reserved but
	// writing 0 is ignored.
	(*tmu).TCR.CKEG = 0;
	// Setting the prescaler.
	(*tmu).TCR.TPSC = prescaler;

	// Loading the structure information.
	timers[timer].callback = callback;
	timers[timer].repeats = repeats;

	// Starting the timer and returning.
	*tstr |= byte;
}
