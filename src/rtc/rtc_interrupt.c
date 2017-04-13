#include <internals/rtc.h>
#include <modules/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	rtc_periodic_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_periodic_interrupt(void)
{
	rtc_cb_interrupt();

	RTC.RCR2->PEF = 0;
}
