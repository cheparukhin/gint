#include <time.h>
#include <internals/time.h>
#include <stdlib.h>

static struct tm tm;

/*
	gmtime()
	Converts calendar time to broken-down time. The returned pointer is
	statically allocated and may be overwritten by any subsequent call to
	a time function.
*/
struct tm *gmtime(const time_t *timeptr)
{
	time_t t = *timeptr;
	div_t d;
	int sec;

	tm.tm_year	= 1970;
	tm.tm_mon	= 0;

	sec = daysInMonth(tm.tm_mon, tm.tm_year) * 24 * 3600;
	while(t >= sec)
	{
		t -= sec;
		if(++tm.tm_mon == 12)
		{
			tm.tm_year++;
			tm.tm_mon = 0;
		}
		sec = daysInMonth(tm.tm_mon, tm.tm_year) * 24 * 3600;
	}
	tm.tm_year	-= 1900;

	d = div(sec, 24 * 3600);
	tm.tm_mday	= d.quot;
	d = div(d.rem, 3600);
	tm.tm_hour	= d.quot;
	d = div(d.rem, 60);
	tm.tm_min	= d.quot;
	tm.tm_sec	= d.rem;

	mktime(&tm);
	return &tm;
}
