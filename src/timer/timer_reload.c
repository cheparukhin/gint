#include <internals/timer.h>
#include <timer.h>

/*
	timer_reload()
	Reloads the given timer with the given constant. Starts the timer if
	it was stopped.
*/
void timer_reload(int timer, int new_delay)
{
	volatile struct mod_tmu *tmu;
	volatile unsigned char *tstr;
	int byte = (1 << timer);
	timer_get(timer, &tmu, &tstr);

	// Setting the constant and the delay.
	tmu->TCOR = new_delay;
	tmu->TCNT = new_delay;

	// Starting the timer.
	*tstr |= byte;
}
