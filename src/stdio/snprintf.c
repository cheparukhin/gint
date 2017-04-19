#include <internals/stdio.h>
#include <stdio.h>
#include <string.h>

/*
	snprintf()
	Prints to a string with a size limit.
*/
int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int x = __printf(size, format, args);
	memcpy(str, __stdio_buffer, x + 1);

	va_end(args);
	return x;
}
