#include <internals/stdio.h>
#include <stdio.h>
#include <string.h>

/*
	sprintf()
	Prints to a string.
*/
int sprintf(char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int x = __printf(0, format, args);
	memcpy(str, __stdio_buffer, x + 1);

	va_end(args);
	return x;
}
