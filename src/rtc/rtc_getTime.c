#include <internals/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	integer()
	Converts a BCD value to an integer.
*/
static int integer8(int bcd)
{
	return (bcd & 0x0f) + 10 * (bcd >> 4);
}
static int integer16(int bcd)
{
	return (bcd & 0xf) + 10 * ((bcd >> 4) & 0xf) + 100 * ((bcd >> 8) & 0xf)
		+ 1000 * (bcd >> 12);
}

/*
	rtc_getTime()
	Reads the current time from the RTC. There is no guarantee that the
	week day is correct (use the time API for that).
*/
rtc_time_t rtc_getTime(void)
{
	volatile struct mod_rtc *rtc = isSH3() ? RTC_SH7705 : RTC_SH7305;
	rtc_time_t time;

	do
	{
		rtc->RCR1.CF = 0;

		time.seconds	= integer8(rtc->RSECCNT.BYTE);
		time.minutes	= integer8(rtc->RMINCNT.BYTE);
		time.hours	= integer8(rtc->RHRCNT.BYTE);
		time.month_day	= integer8(rtc->RDAYCNT.BYTE);
		time.month	= integer8(rtc->RMONCNT.BYTE);
		time.year	= integer16(rtc->RYRCNT.WORD);
		time.week_day	= rtc->RWKCNT;
	}
	while(rtc->RCR1.CF != 0);

	return time;
}
