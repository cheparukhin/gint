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

static int year_day(int year, int year_bcd, int month, int month_day)
{
	int days_in_month[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int leap;
	int hundreds = integer8(year_bcd >> 8);
	int day, i;

	leap = !(year & 3);			// Take multiples of 4
	if(!((year_bcd >> 8) & 0xf)) leap = 0;	// Remove multiples of 100
	if(!(hundreds & 3)) leap = 1;		// Take multiples of 400

	day = leap && (month > 2);
	for(i = 0; i < month - 1; i++) day += days_in_month[i];
	return day + month_day - 1;
}

struct RTCTime rtc_getTime(void)
{
	volatile struct mod_rtc *rtc = isSH3() ? RTC_SH7705 : RTC_SH7305;
	struct RTCTime time;

	time.seconds	= integer8(rtc->RSECCNT.BYTE);
	time.minutes	= integer8(rtc->RMINCNT.BYTE);
	time.hours	= integer8(rtc->RHRCNT.BYTE);
	time.month_day	= integer8(rtc->RDAYCNT.BYTE);
	time.month	= integer8(rtc->RMONCNT.BYTE);
	time.year	= integer16(rtc->RYRCNT.WORD) - 1900;
	time.week_day	= rtc->RWKCNT;
	time.year_day	= year_day(time.year + 1900, rtc->RYRCNT.WORD,
		time.month, time.month_day);

	time.daylight_saving = 0;
	return time;
}
