#ifndef	_STDLIB_H
#define	_STDLIB_H	1

#include <stddef.h>

// Common exit codes.
#define	EXIT_SUCCESS	1
#define	EXIT_FAILURE	0



//---
//	Program exit functions.
//---

/*
	abort()
	Aborts the program execution without calling the exit handlers.
*/
void abort(void);

/*
	exit()
	Stops the program execution with the given status code, after calling
	the exit handlers.

	@arg	status
*/
void exit(int status);



//---
//	Dynamic storage allocation.
//---

/*
	malloc()
	Allocs 'size' bytes and returns a pointer to a free memory area.
	Returns NULL on error.

	@arg	size	Size to allocate, in bytes.

	@return	Memory area address, or NULL.
*/
void *malloc(size_t size);

/*
	calloc()
	Allocs 'n' elements of size 'size' and wipes the memory area. Returns
	NULL on error.

	@arg	n	Element number.
	@arg	size	Element size.

	@return	Memory area address, or NULL.
*/
void *calloc(size_t n, size_t size);

/*
	free()
	Frees a memory block allocated with malloc().

	@arg	ptr	Pointer to free.
*/
void free(void *ptr);



#endif	// _STDLIB_H
