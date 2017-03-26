//---
//
//	standard library module: alloca
//
//	Allows dynamic memory allocation on the stack. Memory is automatically
//	freed when the calling function exits, but this function suffers from
//	risks of stack overflow; make sure you don't inline functions that use
//	alloca or allocate more than a few hundred bytes with it.
//
//---

#ifndef	_ALLOCA_H
#define	_ALLOCA_H

#include <stddef.h>

/*
	alloca()
	Allocates a memory block on the stack.
*/
void *alloca(size_t size);

#define	alloca(size)	__builtin_alloca(size)

#endif	// _ALLOCA_H
