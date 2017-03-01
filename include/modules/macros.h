#ifndef	_MODULES_MACROS_H
#define	_MODULES_MACROS_H

#include <stdint.h>

// Fixed-width types for bit field are totally meaningless.
typedef unsigned uint;

/*
	byte_union()
	Union between an uint8_t 'byte' attribute and a provided bit-field
	structure that describe the contents of the byte.
*/
#define	byte_union(name, fields)		\
	union					\
	{					\
		uint8_t byte;			\
		struct { fields }		\
		__attribute__((packed));	\
	} __attribute__((packed)) name

/*
	word_union()
	Union between an uint16_t 'word' attribute and a provided bit-field
	structure that describe the contents of the word.
*/
#define word_union(name, fields)		\
	union					\
	{					\
		uint16_t word;			\
		struct { fields }		\
		__attribute__((packed));	\
	} __attribute__((packed)) name

/*
	lword_union()
	Union between an uint32_t 'lword' attribute and a provided bit-field
	structure that describe the contents of the longword.
*/
#define	lword_union(name, fields)		\
	union					\
	{					\
		uint32_t lword;			\
		struct { fields }		\
		__attribute__((packed));	\
	} __attribute__((packed)) name

#endif	// _MODULES_MACROS_H
