#include <ctype.h>

// Let's save up some space and readability (that's Cake's idea, it's a bit of
// a preprocessor trick - but a rather nice trick).
#define	r4(x)	(x), (x), (x), (x)
#define	r5(x)	r4(x), (x)
#define	r6(x)	r5(x), (x)
#define	r7(x)	r6(x), (x)
#define r9(x)	r7(x), (x), (x)
#define	r10(x)	r6(x), r4(x)
#define r15(x)	r10(x), r4(x), (x)
#define	r18(x)	r9(x), r9(x)
#define	r20(x)	r10(x), r10(x)

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
	r9(cntrl), r5(cntrl | space), r18(cntrl),
	// Space and some punctuation.
	space | print, r15(punct | print),
	// Decimal digits.
	r10(digit | xdigt | print),
	// Some punctuation.
	r7(punct | print),
	// Uppercase alphabet.
	r6(upper | xdigt | print), r20(upper | print),
	// Other punctuation symbols.
	r6(punct | print),
	// Lowercase alphabet.
	r6(lower | xdigt | print), r20(lower | print),
	// Last punctuation characters and DEL.
	r4(punct | print), cntrl,
};
