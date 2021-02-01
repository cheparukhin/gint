//---
//	gint:src:print - Standard formatted printing functions
//---

#include <gint/std/stdio.h>
#include <gint/kprint.h>
#include <stdarg.h>

/* sprintf() */
GWEAK int sprintf(char *str, char const *format, ...)
{
	va_list args;
	va_start(args, format);

	int count = kvsprint(str, 65536, format, &args);

	va_end(args);
	return count;
}

/* vsprintf() */
GWEAK int vsprintf(char *str, char const *format, va_list args)
{
	return kvsprint(str, 65536, format, &args);
}

/* snprintf() */
GWEAK int snprintf(char *str, size_t n, char const *format, ...)
{
	va_list args;
	va_start(args, format);

	int count = kvsprint(str, n, format, &args);

	va_end(args);
	return count;
}

/* vsprintf() */
GWEAK int vsnprintf(char *str, size_t n, char const *format, va_list args)
{
	return kvsprint(str, n, format, &args);
}
