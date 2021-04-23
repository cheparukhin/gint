//---
//	gint:clock:sleep - Various low-level sleep functions
//---

#include <gint/clock.h>
#include <gint/timer.h>

static void do_sleep(uint64_t delay_us, int spin)
{
	volatile int flag = 0;

	int timer = timer_setup(TIMER_ANY, delay_us, timer_timeout, &flag);
	if(timer < 0) return;

	timer_start(timer);
	if(spin) {
		timer_spinwait(timer);
		timer_stop(timer);
	}
	else timer_wait(timer);
}

/* sleep_us(): Sleep for a fixed duration in microseconds */
void sleep_us(uint64_t delay_us)
{
	do_sleep(delay_us, 0);
}

/* sleep_us_spin(): Actively sleep for a fixed duration in microseconds */
void sleep_us_spin(uint64_t delay_us)
{
	do_sleep(delay_us, 1);
}
