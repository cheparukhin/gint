//---
//	gint:defs:types - Type-related macros
//---

#ifndef GINT_DEFS_TYPES
#define GINT_DEFS_TYPES

#include <defs/attributes.h>
#include <stddef.h>
#include <stdint.h>

//---
//	Const casts
//---

/* const_cast() - perform const casts
   This is intended for initialization purposes only, like "final" in Java.
   This macro is the most generic form without any type inference; using
   const_cint() or const_cptr() may be more convenient for simple types */
#define const_cast(x, T) (*((T *)(&(x))))

/* const_cptr() - perform const casts on pointers */
#define const_cptr(x) (*((void **)(&(x))))

/* const_cint() - perform const casts on integers
   This macro saves you from specifying the integral type that you're
   manipulating, which may avoid errors if the type changes. It will only work
   with the primitive types that are either mentioned in the following list or
   aliased as one of the listed types.
   This is to prevent unforeseen effects of tricks such as typeof(x + 0) that
   promotes various small integers to ints */
#define const_cint(x) (*_Generic((x),					\
	char:			(char *)		(&(x)),		\
	unsigned char:		(unsigned char *)	(&(x)),		\
	short:			(short *)		(&(x)),		\
	unsigned short:		(unsigned short *)	(&(x)),		\
	int:			(int *)			(&(x)),		\
	unsigned int:		(unsigned int *)	(&(x)),		\
	long:			(long *)		(&(x)),		\
	unsigned long:		(unsigned long *)	(&(x)),		\
	long long:		(long long *)		(&(x)),		\
	unsigned long long:	(unsigned long long *)	(&(x))		\
))

//---
//	Structure elements
//----

/* Fixed-width types for bit fields are entirely meaningless */
typedef unsigned int uint;

/* Giving a type to padding bytes is misguiding */
#define pad_nam2(c)	_ ## c
#define pad_name(c)	pad_nam2(c)
#define pad(bytes)	uint8_t pad_name(__COUNTER__)[bytes]

/* byte_union() - union between an uint8_t 'byte' element and a bit field */
#define byte_union(name, fields)		\
	union {					\
		uint8_t byte;			\
		struct { fields } PACKED(1);	\
	} PACKED(1) name

/* word_union() - union between an uint16_t 'word' element and a bit field */
#define word_union(name, fields)		\
	union {					\
		uint16_t word;			\
		struct { fields } PACKED(2);	\
	} PACKED(2) name

/* lword_union() - union between an uint32_t 'lword' element and a bit field */
#define lword_union(name, fields)		\
	union {					\
		uint32_t lword;			\
		struct { fields } PACKED(4);	\
	} PACKED(4) name

#endif /* GINT_DEFS_TYPES */
