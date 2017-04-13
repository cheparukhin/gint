#include <time.h>
#include <rtc.h>

/*
	time()
	Returns the current time as calendar time. If you need a broken-down
	time, either use the RTC API or gmtime(). However, this function is
	already based on mktime() (for hardware reasons) so it would be much
	faster to use the RTC API if possible.
	If timeptr is not NULL, it is set to the current time, that is, the
	value that is returned.
*/
time_t time(time_t *timeptr)
{
	rtc_time_t rtc;
	struct tm tm;
	time_t calendar;

	rtc_getTime(&rtc);
	tm.tm_sec	= rtc.seconds;
	tm.tm_min	= rtc.minutes;
	tm.tm_hour	= rtc.hours;
	tm.tm_mday	= rtc.month_day;
	tm.tm_mon	= rtc.month;
	tm.tm_year	= rtc.year - 1900;

	calendar = mktime(&tm);
	if(timeptr) *timeptr = calendar;
	return calendar;
}
