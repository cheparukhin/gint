//---
//	gint:clock:sleep - Various low-level sleep functions
//---

#include <gint/clock.h>
#include <gint/timer.h>

/* sleep_us(): Sleep for a fixed duration in microseconds */
void sleep_us(uint64_t delay_us)
{
	volatile int flag = 0;

	int timer = timer_setup(TIMER_ANY, delay_us, timer_timeout, &flag);
	if(timer < 0) return;

	timer_start(timer);
	timer_wait(timer);
}
