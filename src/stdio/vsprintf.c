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
	strncpy(str, __stdio_buffer, __stdio_buffer_size);

	return x;
}
