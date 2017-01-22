//---
//
//	standard library module: string
//
//	String manipulation using NUL-terminated byte arrays, without extended
//	characters.
//
//---

#ifndef	_STRING_H
#define	_STRING_H	1

#include <stddef.h>

//---
//	Memory manipulation.
//---

/*
	memcpy()	O(byte_count)
	Copies a memory area. The two areas must not overlap (if they do, use
	memmove()). A smart copy is performed when possible. To enhance
	performance, make sure than destination and source are both 4-aligned.
*/
void *memcpy(void *destination, const void *source, size_t byte_count);

/*
	memset()	O(byte_count)
	Sets the contents of a memory area. A smart copy is performed.
*/
void *memset(void *destination, int byte, size_t byte_count);

/*
	memchr()	O(byte_count)
	Looks for a byte in a memory area. Returns the address of the first
	occurrence if found, NULL otherwise.
*/
void *memchr(const void *area, int byte, size_t byte_count);

/*
	memcmp()	O(byte_count)
	Compares two memory areas. Returns 0 if all bytes are equal in both
	areas, a negative number if the first unequal byte is lower in the
	first area, and a positive number otherwise.
	A smart comparison is performed when possible.
*/
int memcmp(const void *area1, const void *area2, size_t byte_count);



//---
//	String manipulation.
//---

/*
	strlen()	O(len(str))
	Returns the length of a string.
*/
size_t strlen(const char *str);

/*
	strnlen()	O(len(str))
	Returns the minimum of the length of the string and n. This function
	never access more than n bytes at the beginning of the string.
*/
size_t strnlen(const char *str, size_t n);

/*
	strcpy()	O(len(source))
	Copies a string to another.
*/
char *strcpy(char *destination, const char *source);

/*
	strncpy()	O(min(len(source), size))
	Copies part of a string to another.
*/
char *strncpy(char *destination, const char *source, size_t size);

/*
	strchr()	O(len(str))
	Searches a character in a string.
*/
const char *strchr(const char *str, int value);

/*
	strcmp()	O(max(len(str1), len(str2)))
	Compares two strings. Returns 0 if they are identical, a negative
	number if the first unequal byte is lower in str1 than in str2, and a
	positive number otherwise.
*/
int strcmp(const char *str1, const char *str2);

#endif	// _STRING_H
