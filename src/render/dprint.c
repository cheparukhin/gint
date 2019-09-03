#include <gint/display.h>
#include <gint/std/stdio.h>

/* dprint(): Display a formatted string */
void dprint(int x, int y, int fg, int bg, char const *format, ...)
{
	char str[256];
	va_list args;

	va_start(args, format);
	vsnprintf(str, 256, format, args);
	va_end(args);

	dtext(x, y, str, fg, bg);
}
