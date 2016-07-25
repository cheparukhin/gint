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
	memcpy()	O(byte_number)
	Copies a memory area. The two areas must not overlap (if they do, use
	memmove()). A smart copy is performed when possible. To enhance
	performance, make sure than destination and source are both 4-aligned.
*/
void *memcpy(void *destination, const void *source, size_t byte_number);

/*
	memset()	O(byte_number)
	Sets the contents of a memory area. A smart copy is performed.
*/
void *memset(void *destination, int byte, size_t byte_number);



//---
//	String manipulation.
//---

/*
	strlen()	O(len(str))
	Returns the length of a string.
*/
size_t strlen(const char *str);

/*
	strcpy()	O(len(source))
	Copies a string to another.
*/
char *strcpy(char *destination, const char *source);

/*
	strchr()	O(len(str))
	Searches a character in a string.
*/
const char *strchr(const char *str, int value);

/*
	strncpy()	O(min(len(source), size))
	Copies part of a string to another.
*/
char *strncpy(char *destination, const char *source, size_t size);

#endif	// _STRING_H
