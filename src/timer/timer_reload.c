#include <internals/timer.h>
#include <timer.h>

/*
	timer_reload()
	Reloads the given timer with the supplied constant. Starts the timer if
	it was stopped.
*/
void timer_reload(int timer, int new_delay_or_frequency, enum ClockUnit unit)
{
	timer_reload2(timer, clock_setting(new_delay_or_frequency, unit));
}

/*
	timer_reload2()
	Same as timer_reload(), but again uses the native clock count. The
	prescaler may not be changed.
*/
void timer_reload2(int timer, int new_delay)
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
