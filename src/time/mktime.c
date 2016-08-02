#include <time.h>

/*
	isLeap()
	Determines whether the given year is a leap year.
*/
int isLeap(int year)
{
	int leap = !(year & 3);		// Take multiples of 4
	if(!(year % 100)) leap = 0;	// Remove multiples of 100
	if(!(year % 400)) leap = 1;	// Take multiples of 400

	return leap;
}

/*
	daysInMonth()
	Returns number of days for the given month (between 0 and 11) and year.
*/
int daysInMonth(int month, int year)
{
	int days[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	if(month != 1) return days[month];
	return days[month] + isLeap(year);
}

/*
	mktime()
	Computes fields tm_wday and tm_yday using the other fields. Member
	structures outside their range are normalized and tm_isdst is set.
*/
time_t mktime(struct tm *time)
{
	int first, leaps, yr;
	int days, i;
	time_t elapsed;

	// Normalizing time.
	time->tm_min	+= (time->tm_sec / 60);
	time->tm_hour	+= (time->tm_min / 60);
	time->tm_mday	+= (time->tm_hour / 24);
	time->tm_sec	%= 60;
	time->tm_min	%= 60;
	time->tm_hour	%= 24;

	// Normalizing date.
	days = daysInMonth(time->tm_mon, time->tm_year + 1900);
	while(time->tm_mday >= days)
	{
		time->tm_mday -= days;
		if(++time->tm_mon == 12)
		{
			time->tm_mon = 0;
			time->tm_year++;
		}
		days = daysInMonth(time->tm_mon, time->tm_year + 1900);
	}

	// Setting the year day.
	days = 0;
	for(i = 0; i < time->tm_mon; i++)
		days += daysInMonth(i, time->tm_year + 1900);
	time->tm_yday = days + time->tm_mday;

	// Setting the week day. The calendar is periodic over 400 years and
	// 1601-01-01 was a Monday.

	// Years completely elapsed since last 400n + 1 year (1601-2001-etc).
	yr		= (time->tm_year + 1900 - 1) % 400;
	// Leap years in these yr years.
	leaps		= (yr / 4) - (yr >= 100) - (yr >= 200) - (yr >= 300);
	// Days completely elapsed since last 400n + 1 year, 01-01.
	days		= 365 * yr + leaps + time->tm_yday;
	// Current day of week (1 means Monday 1601-01-01).
	time->tm_wday	= (1 + days) % 7;

	// This RTC does not seem to have any DST feature.
	time->tm_isdst = 0;

	if(time->tm_year + 1900 < 1970) return (time_t)-1;

	// 134774 is the number of days between 1601-01-01 and 1970-01-01. Thus
	// days become the number of days elapsed since 1970-01-01.
	days -= 134774;
	// days may become negative, so add the calendar period.
	if(days < 0) days += 146097;

	return (24 * 3600) * days + 3600 * time->tm_hour + 60 * time->tm_min +
		time->tm_sec;
}
