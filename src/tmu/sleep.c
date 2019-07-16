//---
//	gint:clock:sleep - Various low-level sleep functions
//---

#include <gint/clock.h>
#include <gint/timer.h>

/* sleep_us() - sleep for a definite duration in microseconds */
void sleep_us(int tid, int us_delay)
{
	volatile int flag = 0;

	timer_setup(tid, timer_delay(tid, us_delay), 0, timer_timeout, &flag);
	timer_start(tid);

	while(!flag) sleep();
}
