//---
//	gint:std:stdio - a few <stdio.h> functions provided by gint
//---

#ifndef GINT_STD_STDIO
#define GINT_STD_STDIO

#include <stddef.h>
#include <stdarg.h>

/* Formatted printing functions

   These functions implement most of printf(3)'s features, including:
   * Signed and unsigned integer formats (%d, %i, %o, %u, %x, %X)
   * Character, string and pointer formats (%c, %s, %p)
   * Format options (0, #, -, (space), length, precision)
   * Parameter length (hh, h, l, ll, z)
   * Limiting the size of the output and still returning the whole length
   * If kprint_enable_fp() from <gint/kprint.h> is called: floating-point
     formats (%e, %E, %f, %F, %g, %G) (disabled by default to save space)

   They do not support:
   * Hexadecimal floating-point (%a, %A)
   * The strerror() shorthand, since there is no errno (%m)
   * Some size modifiers: long double (L), intmax_t (j), ptrdiff_t (t), and the
     nonstandard synonyms q (ll) and Z (z)
   * Dynamic length field (*)
   * Thousands separators (') and locale-aware digits (I)
   * Nonstandard synonyms %C (%lc) and %S (%ls)

   A new fixed-point format %j has been added; it behaves like %d but includes
   a decimal point. The number of decimal places is specified by the precision
   field. For instance %.3j with 12345 prints "123.45". This can be used for
   decimal fixed-point values. */

/* Print to string from var args */
int sprintf(char *str, char const *format, ...);
/* Print to string from va_list */
int vsprintf(char *str, char const *format, va_list args);
/* Print to string with limited size from var args */
int snprintf(char *str, size_t n, char const *format, ...);
/* Print to string with limited size from va_list */
int vsnprintf(char *str, size_t n, char const *format, va_list args);
/* Print to auto-allocated string */
int asprintf(char **strp, char const *format, ...);
/* Print to auto-allocated string from va_list */
int vasprintf(char **strp, char const *format, va_list args);

#endif /* GINT_STD_STDIO */
