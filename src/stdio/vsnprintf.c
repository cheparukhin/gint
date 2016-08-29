#include <internals/stdio.h>
#include <stdio.h>
#include <string.h>

/*
	vsnprintf()
	The most generic formatted printing function around there.
*/
int vsnprintf(char *str, size_t size, const char *format, va_list args)
{
	int x = __printf(size, format, args);
	memcpy(str, __stdio_buffer, x + 1);

	return x;
}
