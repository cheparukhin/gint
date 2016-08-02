//---
//
//	standard library module: time
//
//	Provides time manipulation and representation functions.
//
//---

#ifndef	_TIME_H
#define	_TIME_H	1

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
	int tm_isdst;		// Always -1 (not available).
};

/*
	clock_t
	Only used by clock().
*/
typedef	signed int clock_t;

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
	difftime()
	Returns the number of seconds between the given points.
*/
double difftime(time_t end, time_t beginning);
// But this macro should do.
#define	difftime(end, beginning) ((double)((end) - (beginning)))



//---
//	Time representation.
//---

//---
//
//---

/*
	mktime()
	Computes fields tm_wday and tm_yday using the other fields. Member
	structures outside their range are normalized and tm_isdst is set.
*/
time_t mktime(struct tm *time);

#endif	// _TIME_H
