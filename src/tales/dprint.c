#include <internals/tales.h>
#include <internals/stdio.h>
#include <stdarg.h>

/*
	dprint()
	Prints a formatted string. Works the same as printf().
*/
void dprint(int x, int y, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	__printf(0, format, args);
	va_end(args);

	dtext(__stdio_buffer, x, y);
}
