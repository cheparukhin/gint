#include <internals/time.h>

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
