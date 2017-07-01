//---
//	Character type functions.
//	Normally this functions need not be linked because there are macros to
//	optimize performance, but we still need them to get some pointers.
//---

// We don't want to include <ctype.h> because it defines all the macros...
#include <stdint.h>
extern uint8_t ctype_classes[0x80];

#define	_inline	__attribute__((always_inline)) inline

_inline int isalnum(int c) {
	return ctype_classes[c] & 0xf0;
}

_inline int isalpha(int c) {
	return ctype_classes[c] & 0x30;
}

_inline int iscntrl(int c) {
	return ctype_classes[c] & 0x01;
}

_inline int isdigit(int c) {
	return ctype_classes[c] & 0x40;
}

_inline int isgraph(int c) {
	return ctype_classes[c] & 0xf4;
}

_inline int islower(int c) {
	return ctype_classes[c] & 0x10;
}

_inline int isprint(int c) {
	return ctype_classes[c] & 0x08;
}

_inline int ispunct(int c) {
	return ctype_classes[c] & 0x04;
}

_inline int isspace(int c) {
	return ctype_classes[c] & 0x02;
}

_inline int isupper(int c) {
	return ctype_classes[c] & 0x20;
}

_inline int isxdigit(int c) {
	return ctype_classes[c] & 0x80;
}

_inline int isascii(int c) {
	return ((unsigned)c <= 0x7f);
}

_inline int isblank(int c) {
	return (c == '\t' || c == ' ');
}

_inline int tolower(int c) {
	return c | isupper(c);
}

_inline int toupper(int c) {
	return c & ~(islower(c) << 1);
}

#undef	_inline
