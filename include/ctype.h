//---
//
//	standard library module: ctype
//
//	Some character manipulation.
//
//---

#ifndef	_CTYPE_H
#define	_CTYPE_H

#include <stdint.h>

extern uint8_t ctype_classes[0x80];

// Character classes.
#define isalnum(c)	(ctype_classes[(int)(c)] & 0xf0)
#define isalpha(c)	(ctype_classes[(int)(c)] & 0x30)
#define iscntrl(c)	(ctype_classes[(int)(c)] & 0x01)
#define isdigit(c)	(ctype_classes[(int)(c)] & 0x40)
#define isgraph(c)	(ctype_classes[(int)(c)] & 0xf4)
#define islower(c)	(ctype_classes[(int)(c)] & 0x10)
#define isprint(c)	(ctype_classes[(int)(c)] & 0x08)
#define ispunct(c)	(ctype_classes[(int)(c)] & 0x04)
#define isspace(c)	(ctype_classes[(int)(c)] & 0x02)
#define isupper(c)	(ctype_classes[(int)(c)] & 0x20)
#define isxdigit(c)	(ctype_classes[(int)(c)] & 0x80)
#define isascii(c)	((unsigned)c <= 0x7f)
#define isblank(c)	(c == '\t' || c == ' ')

// Character manipulation.
#define tolower(c)	((c) | isupper(c))
#define toupper(c)	((c) & ~(islower(c) << 1))

#endif	// _CTYPE_H
