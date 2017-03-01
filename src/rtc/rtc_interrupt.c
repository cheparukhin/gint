#include <internals/rtc.h>
#include <rtc.h>
#include <mpu.h>

int rtc_carry_flag = 0;

/*
	rtc_periodic_interrupt()
	Handles an RTC interrupt by calling the callback.
*/
void rtc_periodic_interrupt(void)
{
	rtc_cb_interrupt();

	volatile struct mod_rtc *RTC = isSH3() ? RTC_SH7705 : RTC_SH7305;
	RTC->RCR2.PEF = 0;
}
