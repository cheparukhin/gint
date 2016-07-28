#include <string.h>

/*
	strchr
	Searches a character in a string.
*/
const char *strchr(const char *str, int value)
{
	while(*str && *str != value) str++;
	return *str ? str : NULL;
}
