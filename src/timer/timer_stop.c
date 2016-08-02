#include <internals/timer.h>
#include <timer.h>

#include <stddef.h>

/*
	timer_stop()
	Stops the given timer. This function may be called even if the timer is
	not running.
*/
void timer_stop(int timer)
{
	volatile unsigned char *tstr;
	int byte = (1 << timer);

	timer_get(timer, NULL, &tstr);
	*tstr &= ~byte;
}
