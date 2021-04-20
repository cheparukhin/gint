//---
//	gint:std:string - a few <string.h> functions provided by gint
//---

#ifndef GINT_STD_STRING
#define GINT_STD_STRING

#include <stddef.h>

/* memcpy(): Copy a chunk of memory to a non-overlapping destination */
void *memcpy(void * restrict dest, void const * restrict src, size_t n);

/* memset(): Fill a chunk of memory with a single byte */
void *memset(void *dest, int byte, size_t n);

/* memcmp(): Compare two chunks of memory */
int memcmp(void const *s1, void const *s2, size_t n);

/* memmove(): Copy a chunk of memory to a possibly overlapping destination */
void *memmove(void *dest, void const *src, size_t n);

/* strlen(): Length of a NUL-terminated string */
size_t strlen(char const *str);

/* strncpy(): Copy a string with a size limit */
char *strncpy(char *dst, char const *src, size_t n);

/* strcat(): Concatenation a string to a pre-allocated space */
char *strcat(char *dest, const char *src);

/* strcmp(): Compare NUL-terminated strings */
int strcmp(char const *s1, char const *s2);

/* strchr(): Find the first occurrence of a character in a string */
char *strchr(char const *s, int c);

/* strrchr(): Find the last occurrence of a character in a string */
char *strrchr(char const *s, int c);

/* strchrnul(): Like strchr(), but returns end-of-string instead of NUL */
char *strchrnul(char const *s, int c);

#endif /* GINT_STD_STRING */
