#include <internals/interrupts.h>
#include <internals/timer.h>
#include <internals/rtc.h>

/*
	inth_timer_underflow()
	Wake up, your timer has expired!
*/
void inth_timer_underflow(uint32_t channel)
{
	timer_interrupt(channel);
}

/*
	inth_rtc_periodic()
	Don't you forget to execute the periodic tasks.
*/
void inth_rtc_periodic(void)
{
	rtc_periodic_interrupt();
}
