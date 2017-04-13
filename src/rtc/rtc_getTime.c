#include <internals/rtc.h>
#include <modules/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	integer8(), integer16() [static]
	Converts a BCD value to an integer.
*/
static int integer8(uint8_t bcd)
{
	return (bcd & 0x0f) + 10 * (bcd >> 4);
}
static int integer16(uint16_t bcd)
{
	return (bcd & 0xf) + 10 * ((bcd >> 4) & 0xf) + 100 * ((bcd >> 8) & 0xf)
		+ 1000 * (bcd >> 12);
}

/*
	rtc_getTime()
	Reads the current time from the RTC. There is no guarantee that the
	week day will contain a correct value (use the time API for that).
*/
void rtc_getTime(rtc_time_t *time)
{
	if(!time) return;

	do
	{
		RTC.RCR1->CF = 0;

		time->seconds	= integer8(RTC.time->RSECCNT.byte);
		time->minutes	= integer8(RTC.time->RMINCNT.byte);
		time->hours	= integer8(RTC.time->RHRCNT .byte);
		time->month_day	= integer8(RTC.time->RDAYCNT.byte);
		time->month	= integer8(RTC.time->RMONCNT.byte);
		time->year	= integer16(RTC.time->RYRCNT.word);
		time->week_day	= RTC.time->RWKCNT;
	}
	while(RTC.RCR1->CF != 0);
}
