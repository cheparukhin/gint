//---
//	gint:kprint - gint's printf(3)-like formatted printer
//---

#ifndef GINT_KPRINT
#define GINT_KPRINT

#include <gint/defs/types.h>
#include <gint/defs/attributes.h>
#include <stdarg.h>

//---
// Formatted printing
//---

/* kvsprint(): Formatted print to string
   This function is similar to vsnprintf(3), except that variadic arguments are
   passed as a pointer to va_list. For standard functions like vsnprintf(3),
   see <stdio.h>. */
size_t kvsprint(char *output, size_t len, char const *format, va_list *args);

//---
// Printer extensions
//---

/* Arguments passed to formatters */
#define KFORMAT_ARGS							\
	GUNUSED int spec, 		/* Specifier letter */		\
	GUNUSED kprint_options_t *opt,	/* Other options in format */	\
	va_list *args			/* Vararg list to read from */

/* kprint_options_t: Standard format options */
typedef struct {
	/* Minimal length of formatted string (padding can be added) */
	uint16_t length;
	/* How much significant characters of data, meaning varies */
	uint16_t precision;

	/* Size specifier for integers (%o, %x, %i, %d, %u), may be one of:
	   (0) char       (8-bit)
	   (1) short     (16-bit)
	   (2) int       (32-bit)
	   (3) long      (32-bit)
	   (4) long long (64-bit) */
	uint8_t size;

	/* (#) Alternative form: base prefixes, decimal point */
	uint alternative :1;
	/* ( ) Add a blank sign before nonnegative numbers */
	uint blank_sign :1;
	/* (+) Always add a sign before a number (overrides ' ') */
	uint force_sign :1;

	/* Alignment options: each option overrides all others before itself
	   NUL By default, align right
	   (0) Left-pad numerical values with zeros
	   (-) Align left by adding space on the right, dropping zeros */
	uint8_t alignment;

} GPACKED(2) kprint_options_t;

/* kprint_formatter_t: Type of format function */
typedef void (*kprint_formatter_t)(KFORMAT_ARGS);

/* kprint_register(): Register a formatter

   The formatter designated by the specified lowercase letter (eg 'q') is
   registered. It will handle formats for both the lowercase and uppercase
   formats (eg "%q" and "%Q").

   Default formatters can be overridden (although not advised), but formatters
   for "h" and "l" and "z" cannot be set because these letters are used as size
   specifiers (eg in "%hd" or "%zu").

   A formatter can be removed or disabled by registering NULL.

   @spec     Specifier to use, should be a lowercase letter
   @kformat  Format function */
void kprint_register(int spec, kprint_formatter_t kformat);

//---
// Helper functions for formatters
//---

/* kprint_geometry_t: General geometry for a format
   This helper defines the structure of a format as follows:

                    sign v     |< zeros >| |< content >|
       _ _ _ _ _ _ _ _ _ + 0 x 0 0 0 0 0 0 8 a c 7 . 3 c _ _ _ _ _ _ _ _ _ _
       |< left_spaces >|   ^^^ prefix                    |< right_spaces  >|

    The sign character is absent if sign=0, the prefix is specified by length
    and is also absent if prefix=0. */
typedef struct
{
	uint16_t left_spaces;	/* Spaces before content */
	uint8_t  sign;		/* Sign character (NUL, ' ', '+' or '-') */
	uint8_t  prefix;	/* Base prefix ('0', '0x', etc) length */
	uint16_t zeros;		/* For integer displays, number of zeros */
	uint16_t content;	/* Content length in bytes */
	uint16_t right_spaces;	/* Spaces after content */

	/* Style of display:
	     KPRINT_GENERIC    Sign ignored, 0-padding ignored
	     KPRINT_INTEGER    .precision causes 0-padding
	     KPRINT_NUMERICAL  No effect */
	enum { KPRINT_GENERIC = 0, KPRINT_INTEGER, KPRINT_NUMERICAL } style;

} GPACKED(2) kprint_geometry_t;

/* kformat_geometry(): Calculate the geometry of a format

   The caller provides as input:
   * opt, as passed by the main kprint() routine
   * g->prefix, the length of the desired prefix (if unused, 0)
   * g->content, the natural content length for the provided data
   * g->sign, the sign of the input ("+" or "-"); for KPRINT_GENERIC, 0

   This function outputs:
   * g->sign, which can be changed to " " or NUL depending on opt
   * All other fields of g that are not part of the input

   The algorithm for laying out the format is as follows.
   1. For numerical and integer formats, turn a "+" sign into " " if
      opt->blank_sign is set, "+" if opt->force_sign is set, NUL otherwise.
   2. Compute the total amount of padding needed to reach opt->length.
   3. In integer style, if a precision is specified and more than content
      length plus sign and prefix, turn some padding into zeros.
   4. If numerical and integer styles, if opt->alignment == '0' turn all the
      padding into zeros.
   5. Turn remaining padding into spaces at the left (if opt->alignment == NUL)
      or right (if opt->alignment == '-').

   @opt  Options provided by the main kprint() routine
   @g    Format geometry (input/output) */
void kformat_geometry(kprint_options_t *opt, kprint_geometry_t *g);

#endif /* GINT_KPRINT */
