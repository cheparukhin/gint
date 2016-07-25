//---
//
//	standard library module: alloca
//
//	Allows dynamic memory allocation on the stack. Memory is automatically
//	freed when the calling function exits.
//
//---

#ifndef	_ALLOCA_H
#define	_ALLOCA_H	1

#include <stddef.h>

/*
	alloca()
	Allocates a memory block on the stack.
*/
void *alloca(size_t size);

#define	alloca(size)	__builtin_alloca(size)

#endif	// _ALLOCA_H
