#include <internals/rtc.h>
#include <modules/rtc.h>
#include <rtc.h>
#include <mpu.h>

/*
	bcd8(), bcd16() [static]
	Converts an integer to a BCD value.
*/
static uint8_t bcd8(int integer)
{
	integer %= 100;
	return ((integer / 10) << 4) | (integer % 10);
}
static uint16_t bcd16(int integer)
{
	integer %= 10000;
	return (bcd8(integer / 100) << 8) | bcd8(integer % 100);
}

/*
	rtc_setTime()
	Sets the time in the RTC registers. The week day is set to 0 if greater
	than 6. Other fields are not checked.
*/
void rtc_setTime(const rtc_time_t *time)
{
	if(!time) return;
	int wday = (time->week_day < 7) ? (time->week_day) : (0);

	do
	{
		RTC.RCR1->CF = 0;

		RTC.time->RSECCNT.byte	= bcd8(time->seconds);
		RTC.time->RMINCNT.byte	= bcd8(time->minutes);
		RTC.time->RHRCNT .byte	= bcd8(time->hours);
		RTC.time->RDAYCNT.byte	= bcd8(time->month_day);
		RTC.time->RMONCNT.byte	= bcd8(time->month);
		RTC.time->RYRCNT .word	= bcd16(time->year);
		RTC.time->RWKCNT	= wday;
	}
	while(RTC.RCR1->CF != 0);
}
