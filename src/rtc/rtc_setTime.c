#include <internals/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	bcd()
	Converts an integer to a BCD value.
*/
static int bcd8(int integer)
{
	integer %= 100;
	return ((integer / 10) << 4) | (integer % 10);
}
static int bcd16(int integer)
{
	integer %= 10000;
	return (bcd8(integer / 100) << 8) | bcd8(integer % 100);
}

void rtc_setTime(struct RTCTime time)
{
	volatile struct mod_rtc *rtc = isSH3() ? RTC_SH7705 : RTC_SH7305;

	rtc->RSECCNT.BYTE	= bcd8(time.seconds);
	rtc->RMINCNT.BYTE	= bcd8(time.minutes);
	rtc->RHRCNT.BYTE	= bcd8(time.hours);
	rtc->RDAYCNT.BYTE	= bcd8(time.month_day);
	rtc->RMONCNT.BYTE	= bcd8(time.month);
	rtc->RYRCNT.WORD	= bcd16(time.year + 1900);
	rtc->RWKCNT		= time.week_day < 7 ? time.week_day : 0;
}
