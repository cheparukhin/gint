#include <internals/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	rtc_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_interrupt(void)
{
	rtc_cb_interrupt();

	volatile struct mod_rtc *RTC = isSH3() ? RTC_SH7705 : RTC_SH7305;
	RTC->RCR2.PEF = 0;
}
