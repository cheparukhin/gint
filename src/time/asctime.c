#include <time.h>

static char str[26];

/*
	asctime()
	Converts broken-down time to string representation on the form
	"Wed Jun 30 21:49:08 1993\n". The returned string is statically
	allocated and may be overwritten by any subsequent call to a time
	function.
*/
char *asctime(const struct tm *time)
{
	const char *days[] = {
		"Sun",	"Mon",	"Tue",	"Wed",	"Thu",	"Fri",	"Sat"
	}, *months[] = {
		"Jan",	"Feb",	"Mar",	"Apr",	"May",	"Jun",	"Jul",	"Aug",
		"Sep",	"Oct",	"Nov",	"Dec"
	};
	int year = time->tm_year + 1900;

	str[0]	= days[time->tm_wday][0];
	str[1]	= days[time->tm_wday][1];
	str[2]	= days[time->tm_wday][2];
	str[3]	= ' ';

	str[4]	= months[time->tm_mon][0];
	str[5]	= months[time->tm_mon][1];
	str[6]	= months[time->tm_mon][2];
	str[7]	= ' ';

	str[8]	= '0' + (time->tm_mday / 10);
	str[9]	= '0' + (time->tm_mday % 10);
	str[10]	= ' ';

	str[11]	= '0' + (time->tm_hour / 10);
	str[12]	= '0' + (time->tm_hour % 10);
	str[13] = ':';
	str[14]	= '0' + (time->tm_min / 10);
	str[15]	= '0' + (time->tm_min % 10);
	str[16] = ':';
	str[17]	= '0' + (time->tm_sec / 10);
	str[18]	= '0' + (time->tm_sec % 10);
	str[19] = ' ';

	str[20] = '0' + (year / 1000);
	year %= 1000;
	str[21] = '0' + (year / 100);
	year %= 100;
	str[22] = '0' + (year / 10);
	str[23] = '0' + (year % 10);

	str[24] = '\n';
	str[25] = 0;

	return str;
}
