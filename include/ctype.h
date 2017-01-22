//---
//
//	standard library module: ctype
//
//	Some character manipulation.
//
//---

#ifndef	_CTYPE_H
#define	_CTYPE_H 1

#include <stdint.h>

//---
//	Character classes.
//---

extern uint8_t ctype_classes[0x80];

__attribute__((always_inline)) static inline int isalnum(int c) {
	return ctype_classes[c] & 0xf0;
}

__attribute__((always_inline)) static inline int isalpha(int c) {
	return ctype_classes[c] & 0x30;
}

__attribute__((always_inline)) static inline int iscntrl(int c) {
	return ctype_classes[c] & 0x01;
}

__attribute__((always_inline)) static inline int isdigit(int c) {
	return ctype_classes[c] & 0x40;
}

__attribute__((always_inline)) static inline int isgraph(int c) {
	return ctype_classes[c] & 0xf4;
}

__attribute__((always_inline)) static inline int islower(int c) {
	return ctype_classes[c] & 0x10;
}

__attribute__((always_inline)) static inline int isprint(int c) {
	return ctype_classes[c] & 0x08;
}

__attribute__((always_inline)) static inline int ispunct(int c) {
	return ctype_classes[c] & 0x04;
}

__attribute__((always_inline)) static inline int isspace(int c) {
	return ctype_classes[c] & 0x02;
}

__attribute__((always_inline)) static inline int isupper(int c) {
	return ctype_classes[c] & 0x20;
}

__attribute__((always_inline)) static inline int isxdigit(int c) {
	return ctype_classes[c] & 0x80;
}

__attribute__((always_inline)) static inline int isascii(int c) {
	return (c >= 0 && c <= 0x7f);
}

__attribute__((always_inline)) static inline int isblank(int c) {
	return (c == '\t' || c == ' ');
}

//---
//	Character manipulation.
//---

__attribute__((always_inline)) static inline int tolower(int c) {
	return c | isupper(c);
}

__attribute__((always_inline)) static inline int toupper(int c) {
	return c & ~(islower(c) << 1);
}

#endif	// _CTYPE_H
