//---
//
//	standard library module: stdlib
//
//	Provides standard functionalities such as dynamic allocation,
//	string/numeric conversion, and abort calls.
//
//---

#ifndef	_STDLIB_H
#define	_STDLIB_H

//---
//	Common definitions.
//---

#include <stddef.h>
#include <limits.h>

// Common exit codes.
#define	EXIT_SUCCESS	1
#define	EXIT_FAILURE	0

// Number of atexit() registrations guaranteed.
#ifndef	ATEXIT_MAX
#define	ATEXIT_MAX	16
#endif

// Maximum value returned by rand().
#define	RAND_MAX	INT_MAX

// Integer division result.
typedef struct
{
	int quot, rem;
} div_t;
typedef struct
{
	long quot, rem;
} ldiv_t;



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
*/
void exit(int status);

/*
	atexit()
	Registers a function to be called at normal program termination.
*/
int atexit(void (*function)(void));



//---
//	Dynamic storage allocation.
//---

/*
	malloc()
	Allocates 'size' bytes and returns a pointer to a free memory area.
	Returns NULL on error.
*/
void *malloc(size_t size);

/*
	calloc()
	Allocates 'n' elements of size 'size' and wipes the memory area.
	Returns NULL on error.
*/
void *calloc(size_t n, size_t size);

/*
	realloc()
	Reallocates a memory block and moves its data.
*/
void *realloc(void *ptr, size_t size);

/*
	free()
	Frees a memory block allocated by malloc(), calloc() or realloc().
*/
void free(void *ptr);



//---
//	Random number generation.
//---

/*
	rand()
	Returns a pseudo-random number.
*/
int rand(void);

/*
	srand()
	Changes the seed used by rand().
*/
void srand(unsigned int seed);

//---
//	Integer arithmetic.
//---

/*
	abs()
	Returns the absolute value of an integer.
*/
int abs(int x);
// Use a macro instead, when possible.
#define	abs(x)	((x) < 0 ? -(x) : (x))

/*
	labs()
	Returns the absolute value of a long integer.
*/
long labs(long x);
// Use a macro instead.
#define	labs(x)	((x) < 0 ? -(x) : (x))

/*
	div()
	Computes the integer division of numerator by denominator.
*/
div_t div(int numerator, int denominator);

/*
	ldiv()
	Computes the integer division of two long integers.
*/
ldiv_t ldiv(long numerator, long denominator);

#endif	// _STDLIB_H
