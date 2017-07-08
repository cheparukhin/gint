//---
//
//	gint libc module: math
//
//	Provides mathematical functions as well as a few useful extensions.
//
//---

#ifndef	_MATH_H
#define	_MATH_H

#include <stdint.h>

//---
//	Function extensions
//---

/*
	qdiv()
	Quickly divides by predefined integers using a 64-bit multiplication
	technique. These functions should be ~10 times faster than dividing
	using opeator "/".
*/

typedef struct qdiv_t
{
	uint32_t q;	/* Quotient */
	uint32_t r;	/* Remainer */

} __attribute__((packed, aligned(4))) qdiv_t;

qdiv_t qdiv(uint32_t n, uint32_t divider, uint32_t reciprocal);

/* Predefined magic numbers */
#define	qdiv_3(n)	qdiv(n,    3, 0x55555556)
#define	qdiv_5(n)	qdiv(n,    5, 0x33333334)
#define	qdiv_10(n)	qdiv(n,   10, 0x1999999a)
#define	qdiv_100(n)	qdiv(n,  100, 0x028f5c29)
#define	qdiv_1000(n)	qdiv(n, 1000, 0x00418938)

#endif	// _MATH_H
