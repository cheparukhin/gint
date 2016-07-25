//---
//
//	standard library module: stdio
//
//	Handles most input/output for the program. This module does not
//	interact with the file system directly.
//
//---

#ifndef	_STDIO_H
#define	_STDIO_H	1

#include <stddef.h>
#include <stdarg.h>

//---
//	Formatted printing.
//---

/*
	sprintf()
	Prints to a string.
*/
int sprintf(char *str, const char *format, ...);

/*
	snprintf()
	Prints to a string with a size limit.
*/
int snprintf(char *str, size_t size, const char *format, ...);

/*
	vsprintf()
	Prints to a string from an argument list.
*/
int vsprintf(char *str, const char *format, va_list args);

/*
	vsnprintf()
	The most generic formatted printing function around there.
*/
int vsnprintf(char *str, size_t size, const char *format, va_list args);

#endif	// _STDIO_H
