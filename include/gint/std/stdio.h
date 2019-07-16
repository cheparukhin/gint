//---
//	gint:std:stdio - a few <stdio.h> functions provided by gint
//---

#ifndef GINT_STD_STDIO
#define GINT_STD_STDIO

#include <stddef.h>
#include <stdarg.h>

/* Formatted printing functions
   These functions implement most of printf()'s features, except:
   * Large parameters (ll)
   * Floating-point (%e, %E, %f, %F, %g, %G, %a, %A) */

/* Print to string from var args */
int sprintf(char *str, char const *format, ...);
/* Print to string from va_list */
int vsprintf(char *str, char const *format, va_list args);
/* Print to string with limited size from var args */
int snprintf(char *str, size_t n, char const *format, ...);
/* Print to string with limited size from va_list */
int vsnprintf(char *str, size_t n, char const *format, va_list args);

#endif /* GINT_STD_STDIO */
