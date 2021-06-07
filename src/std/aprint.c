//---
//	gint:src:aprint - Allocating extensions to formatted printing
//---

#include <stdio.h>
#include <gint/std/stdlib.h>
#include <gint/defs/attributes.h>

/* vasprintf() */
GWEAK int vasprintf(char **strp, char const *format, va_list args1)
{
	va_list args2;
	va_copy(args2, args1);

	int count = vsnprintf(NULL, 0, format, args1);
	va_end(args1);

	char *str = malloc(count + 1);
	if(str) count = vsnprintf(str, count + 1, format, args2);
	va_end(args2);

	if(!str) return -1;
	*strp = str;
	return count;
}

/* asprintf() */
GWEAK int asprintf(char **strp, char const *format, ...)
{
	va_list args;
	va_start(args, format);

	int count = vasprintf(strp, format, args);

	va_end(args);
	return count;
}

