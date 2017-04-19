//---
//
//	standard library module: time
//
//	Provides time manipulation and representation functions.
//
//---

#ifndef	_TIME_H
#define	_TIME_H

#include <stddef.h>

//---
//	Some related types.
//---

/*
	struct tm
	Represents a point in time and gives some date information.
*/
struct tm
{
	int tm_sec;		// Seconds in range 0-59
	int tm_min;		// Minutes in range 0-59
	int tm_hour;		// Hours in range 0-23
	int tm_mday;		// Day of month in range 1-31
	int tm_mon;		// Month in range 0-11
	int tm_year;		// Number of years since 1900
	int tm_wday;		// Day of week in range 0(Sunday)-6(Saturday).
	int tm_yday;		// Day of the year in range 0-365.
	int tm_isdst;		// This will always be 0.
};

/*
	clock_t
	Only used by clock().
*/
typedef signed int clock_t;

/*
	time_t
	Number of seconds elapsed since 1970-01-01 00:00:00.
*/
typedef signed int time_t;



//---
//	Time access.
//---

/*
	clock()
	Should return elapsed CPU time since beginning of program execution.
	This is currently not implemented and returns -1.
*/
clock_t clock(void);

/*
	time()
	Returns the current time as calendar time. If you need a broken-down
	time, either use the RTC API or gmtime(). However, this function is
	already based on mktime() (for hardware reasons) so it would be much
	faster to use the RTC API if possible.
	If timeptr is not NULL, it is set to the current time, that is, the
	value that is returned.
*/
time_t time(time_t *timeptr);

/*
	difftime()
	Returns the number of seconds between the given points.
*/
double difftime(time_t end, time_t beginning);
// But this macro should do.
#define	difftime(end, beginning) ((double)((end) - (beginning)))



//---
//	Time representation.
//---

/*
	asctime()
	Converts broken-down time to string representation on the form
	"Wed Jun 30 21:49:08 1993\n". The returned string is statically
	allocated and may be overwritten by any subsequent call to a time
	function.
*/
char *asctime(const struct tm *time);

/*
	ctime()
	Converts calendar time to string representation on the form
	"Wed Jun 30 21:49:08 1993\n". The returned string is statically
	allocated and may be overwritten by any subsequent call to a time
	function.
*/
char *ctime(const time_t *timer);



//---
//	Time conversion.
//---

/*
	mktime()
	Converts broken-down time to calendar time. Computes structure fields
	tm_wday and tm_yday using the other fields. Member structures outside
	their range are normalized (e.g. 40 October becomes 9 November) and
	tm_isdst is set.
*/
time_t mktime(struct tm *time);

/*
	gmtime()
	Converts calendar time to broken-down time. The returned pointer is
	statically allocated and may be overwritten by any subsequent call to
	a time function.
*/
struct tm *gmtime(const time_t *t);

#endif	// _TIME_H
