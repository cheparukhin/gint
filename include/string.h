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
	strlen()	O(length)
	Returns the length of a string.
*/
size_t strlen(const char *str);

#endif	// _STRING_H
