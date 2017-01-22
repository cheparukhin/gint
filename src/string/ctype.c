#include <ctype.h>

enum {
	cntrl	= 0x01,
	space	= 0x02,
	punct	= 0x04,
	print	= 0x08,
	upper	= 0x20,
	lower	= 0x10,
	digit	= 0x40,
	xdigt	= 0x80,
};

uint8_t ctype_classes[0x80] = {
	// Control characters.
	cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl,
	cntrl | space,	// Tabulation
	cntrl | space, cntrl | space, cntrl | space, cntrl | space,
	cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl,
	cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl, cntrl,

	// Space and some punctuation.
	space | print,
	punct | print, punct | print, punct | print, punct | print,
	punct | print, punct | print, punct | print, punct | print,
	punct | print, punct | print, punct | print, punct | print,
	punct | print, punct | print, punct | print,

	// Decimal digits.
	digit | xdigt | print, digit | xdigt | print, digit | xdigt | print,
	digit | xdigt | print, digit | xdigt | print, digit | xdigt | print,
	digit | xdigt | print, digit | xdigt | print, digit | xdigt | print,
	digit | xdigt | print,

	// Some punctuation.
	punct | print, punct | print, punct | print, punct | print,
	punct | print, punct | print, punct | print,

	// Uppercase alphabet.
	upper | xdigt | print, upper | xdigt | print, upper | xdigt | print,
	upper | xdigt | print, upper | xdigt | print, upper | xdigt | print,
	upper | print, upper | print, upper | print, upper | print,
	upper | print, upper | print, upper | print, upper | print,
	upper | print, upper | print, upper | print, upper | print,
	upper | print, upper | print, upper | print, upper | print,
	upper | print, upper | print, upper | print, upper | print,

	// Other punctuation symbols.
	punct | print, punct | print, punct | print, punct | print,
	punct | print, punct | print,

	// Lowercase alphabet.
	lower | xdigt | print, lower | xdigt | print, lower | xdigt | print,
	lower | xdigt | print, lower | xdigt | print, lower | xdigt | print,
	lower | print, lower | print, lower | print, lower | print,
	lower | print, lower | print, lower | print, lower | print,
	lower | print, lower | print, lower | print, lower | print,
	lower | print, lower | print, lower | print, lower | print,
	lower | print, lower | print, lower | print, lower | print,

	// Last punctuation characters and DEL.
	punct | print, punct | print, punct | print, punct | print,
	cntrl
};
