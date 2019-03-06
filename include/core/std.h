//---
//	gint:core:std - a few standard functions implemented in gint
//
//	There are few enough of them that it felt unnecessary to use a full-
//	fledged standard library.
//---

#ifndef GINT_CORE_STD
#define GINT_CORE_STD

#include <gint/defs/types.h>
#include <stdarg.h>

/* memcpy() - copy a chunk of memory to a non-overlapping destination */
void *memcpy(void * restrict dest, const void * restrict src, size_t n);

/* memset() - fill a chunk of memory with a single byte */
void *memset(void *dest, int byte, size_t n);

/* strlen() - length of a NUL-terminated string */
size_t strlen(const char *str);

/* vsprintf() - an almost-empty subset of the real one */
void vsprintf(char *str, const char *format, va_list args);

/* sprintf() - an almost-empty subset of the real one */
void sprintf(char *str, const char *format, ...);

#endif /* GINT_CORE_STD */
