#include <internals/stdio.h>
#include <stdio.h>
#include <string.h>

/*
	vsprintf()
	Prints to a string from an argument list.
*/
int vsprintf(char *str, const char *format, va_list args)
{
	int x = __printf(0, format, args);
	memcpy(str, __stdio_buffer, x + 1);

	return x;
}
