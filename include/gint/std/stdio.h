//---
//	gint:std:stdio - a few <stdio.h> functions provided by gint
//---

#ifndef GINT_STD_STDIO
#define GINT_STD_STDIO

#include <stddef.h>
#include <stdarg.h>

/* Formatted printing functions

   These functions implement most of printf()'s features, including:
   * Signed and unsigned integer formats (%d, %i, %o, %u, %x, %X)
   * Character, string and pointer formats (%c, %s, %p)
   * Format options (0, #, -, (space), length, precision)
   * Parameter size (hh, h, l, ll)
   * Limiting the size of the output and still returning the whole length

   They do not support:
   * Floating-point (%e, %E, %f, %F, %g, %G, %a, %A)
   * Exotic integer types (intmax_t) or features (thousands separators)

   A new fixed-point format %j has been added; it behaves like %d but includes
   a decimal point. The number of decimal places is specified by the precision
   field. */

/* Print to string from var args */
int sprintf(char *str, char const *format, ...);
/* Print to string from va_list */
int vsprintf(char *str, char const *format, va_list args);
/* Print to string with limited size from var args */
int snprintf(char *str, size_t n, char const *format, ...);
/* Print to string with limited size from va_list */
int vsnprintf(char *str, size_t n, char const *format, va_list args);

#endif /* GINT_STD_STDIO */
