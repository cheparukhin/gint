//---
//	The stdio formatted printer.
//
//	Things to change when creating new formats:
//	-> List of prototypes at the end of the definition section
//	-> Formatted table just below the list of prototypes
//	-> List of non-numerical specifiers in kformat_geometry()
//---

#include <gint/std/stdio.h>
#include <gint/std/string.h>

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>
#include <gint/defs/util.h>

#include <stdarg.h>

//---
//	kprint() definitions
//---

#define KPRINT_BUFSIZE 256
#define KFORMAT_ARGS \
	GUNUSED int spec, GUNUSED struct kprint_options *opt, va_list *args

/* Current position in the output buffer */
static char *kprint_ptr = NULL;
/* Limit of the output buffer */
static char *kprint_limit = NULL;

/* Total number of characters written for this call to kprint() */
static size_t kprint_count = 0;
/* Address of the current output buffer */
static char *kprint_buffer = NULL;
/* Internal buffer, used in many cases */
static char kprint_internal[KPRINT_BUFSIZE];

/* Output kind for this execution of kprint() */
static enum {
	/* Output is user-provided string, then internal buffer for the rest */
	KPrintOutputString = 0,
	/* Output is a device, file descriptor, whatever */
//	KprintOutputDevice = 1,
} kprint_type;

/*
	struct kprint_options -- standard format options
*/
struct kprint_options
{
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

} GPACKED(2);

typedef void (*kprint_formatter_t)(KFORMAT_ARGS);

/* Default formatters */
static void kformat_char (KFORMAT_ARGS);
static void kformat_int  (KFORMAT_ARGS);
static void kformat_uint (KFORMAT_ARGS);
static void kformat_ptr  (KFORMAT_ARGS);
static void kformat_str  (KFORMAT_ARGS);
static void kformat_fixed(KFORMAT_ARGS);

/* Formatter functions for a..z */
kprint_formatter_t kprint_formatters[26] = {
	/* Standard formats plus the 'j' for fixed-point integers in base 10 */
	NULL,		NULL,		kformat_char,	kformat_int,
	NULL,		NULL,		NULL,		NULL,
	kformat_int,	kformat_fixed,	NULL,		NULL,
	NULL,		NULL,		kformat_uint,	kformat_ptr,
	NULL,		NULL,		kformat_str,	NULL,
	kformat_uint,	NULL,		NULL,		kformat_uint,
	NULL,		NULL,
};

//---
//	Output functions
//---

/* kprint_flush(): Flush the buffer when it's full */
static void kprint_flush(void)
{
	/* Update the number of flushed characters */
	kprint_count += kprint_ptr - kprint_buffer;

	if(kprint_type != KPrintOutputString) return;

	/* Make sure to write a NUL at the end of the string, even if
	   it means overriding one of the generated characters */
	if(kprint_buffer != kprint_internal)
	{
		char *nul = min(kprint_ptr, kprint_limit - 1);
		*nul = 0x00;
	}

	/* Switch to the internal buffer now that the output string has
	   been filled */
	kprint_buffer = kprint_internal;
	kprint_limit  = kprint_internal + KPRINT_BUFSIZE;
	kprint_ptr    = kprint_buffer;
}

/* kprint_out(): Output a single character to the buffer */
GINLINE static void kprint_out(int byte)
{
	if(kprint_ptr >= kprint_limit) kprint_flush();
	*kprint_ptr++ = byte;
}

/* kprint_outn(): Output the same character <n> times */
GINLINE static void kprint_outn(int byte, size_t n)
{
	while(n--) kprint_out(byte);
}

//---
//	Parsing helpers
//---

/* kprint_opt(): Parse option strings */
struct kprint_options kprint_opt(char const **options_ptr)
{
	/* No options enabled by default, set the size to int */
	struct kprint_options opt = { .size = 2 };

	/* This function acts as a deterministic finite automaton */
	enum {
		basic,      /* Reading option characters */
		length,     /* Reading length digits */
		precision,  /* Reading precision digits */
	} state = basic;

	char const *options = *options_ptr;

	for(int c; (c = *options); options++)
	{
		int c_low = c | 0x20;
		if(c_low >= 'a' && c_low <= 'z' && c != 'h' && c != 'l') break;

		if(c == '.')
		{
			state = precision;
			continue;
		}
		else if(state == length && c >= '0' && c <= '9')
		{
			opt.length = opt.length * 10 + (c - '0');
			continue;
		}
		else if(state == precision && c >= '0' && c <= '9')
		{
			opt.precision = opt.precision * 10 + (c - '0');
			continue;
		}

		/* Remain in basic state mode */
		state = basic;

		if(c == '#') opt.alternative = 1;
		if(c == ' ') opt.blank_sign = 1;
		if(c == '+') opt.force_sign = 1;

		/* Alignment options, including priority */
		if((c == '-' || c == '0') && opt.alignment != '0')
		{
			opt.alignment = c;
		}

		/* Data size */
		if(c == 'h') opt.size--;
		if(c == 'l') opt.size++;

		if(c >= '1' && c <= '9') state = length, options--;
	}

	*options_ptr = options;
	return opt;
}

//---
//	Base formatted printer
//---

/* kprint(): The kernel's formatted printer */
void kprint(char const *format, va_list *args)
{
	int c, spec;
	struct kprint_options opt;

	while((c = *format++))
	{
		if(c != '%')
		{
			kprint_out(c);
			continue;
		}

		if(!(c = *format)) break;

		/* '%' character */
		if(c == '%')
		{
			kprint_out('%');
			continue;
		}

		opt = kprint_opt(&format);
		spec = *format++;

		if(spec)
		{
			int id = (spec | 0x20) - 'a';
			kprint_formatters[id](spec, &opt, args);
		}
	}

	kprint_flush();
}

/* kvsprint(): Formatted print to string */
size_t kvsprint(char *output, size_t length, char const *format, va_list *args)
{
	kprint_buffer = output;
	kprint_limit  = output + length;

	kprint_count = 0;
	kprint_ptr   = kprint_buffer;
	kprint_type  = KPrintOutputString;

	kprint(format, args);
	return kprint_count;
}

//---
//	Formatter helpers
//---

/* struct geometry: General geometry for a format */
struct geometry
{
	uint16_t left_spaces;	/* Spaces before content */
	uint8_t  sign;		/* Sign character (NUL, ' ', '+' or '-') */
	uint8_t  prefix;	/* Base prefix ('0', '0x', etc) length */
	uint16_t zeros;		/* For numerical displays, number of zeros */
	uint16_t content;	/* Content length in bytes */
	uint16_t right_spaces;	/* Spaces after content */

} GPACKED(2);

/* kformat_geometry(): Calculate the geometry of a format

   The caller must provide the [prefix] and [content] lengths in the geometry
   structure. Additionally, a sign must be indicated: either '+' if the
   formatted value is nonnegative, or '-' otherwise. The sign might be updated
   by this function, but not the other two fields.

   This function is not *really* isolated from the standard kformat functions
   (as would a precise API be), because the meaning of each options and the
   process of resolving them varies so much. */
static void kformat_geometry(int spec, struct kprint_options *opt,
	struct geometry *g)
{
	/* Determining whether we are in numerical. The blacklist approach
	   allows custom specifiers which call this function to enable zeros if
	   they like (they can disable them by un-setting opt->alignment if
	   it's '0' in any case) */
	int numerical = (spec != 'c' && spec != 'p' && spec != 's');

	ssize_t padding;

	/* Sign character (no discussion required for negative values) */
	if(numerical && g->sign == '+')
	{
		g->sign = 0;
		if(opt->blank_sign) g->sign = ' ';
		if(opt->force_sign) g->sign = '+';
	}

	g->zeros = 0;

	padding = opt->length - !!g->sign - g->prefix
		- max(g->content, opt->precision);
	if(padding < 0) padding = 0;

	/* In numerical modes, precision forces zeros */
	if(numerical && opt->precision)
	{
		if(opt->alignment == '0') opt->alignment = 0;

		ssize_t zeros = opt->precision - g->content;
		if(zeros > 0) g->zeros = zeros;
	}

	if(opt->alignment == '0')
	{
		/* Zeros are only allowed in numerical modes */
		if(numerical) g->zeros = padding;
		else g->left_spaces = padding;
	}
	else if(opt->alignment == '-')
	{
		g->right_spaces = padding;
	}
	else
	{
		g->left_spaces = padding;
	}
}

/* digits_10(): Generate digits in base 10
   Fills up the provided digit string from least significant to most
   significant digit, not adding zeros except if argument is zero. Returns the
   number of digits. No NUL terminator is added. */
static int digits_10(char *str, uint64_t n)
{
	int digits = 0;

	while(n || !digits)
	{
		/* TODO: Use qdiv() in kprint's digits_10() */
		str[digits++] = (n % 10) + '0';
		n /= 10;
	}
	return digits;
}

/* digits_16(): Generate digits in base 16 */
static int digits_16(char *str, int uppercase, uint64_t n)
{
	char *hex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	int digits = 0;

	while(n || !digits)
	{
		str[digits++] = hex[n & 0xf];
		n >>= 4;
	}
	return digits;
}

/* digits_8(): Generate digits in base 8 */
static int digits_8(char *str, uint64_t n)
{
	int digits = 0;

	while(n || !digits)
	{
		str[digits++] = (n & 0x7) + '0';
		n >>= 3;
	}
	return digits;
}

//---
//	Loading helpers
//---

static int64_t load_i(int size, va_list *args)
{
	/* All smaller types are promoeted to int so we can't read the
	   explicitly. They have been converted and sign-extended we don't need
	   to care what their size is, the result will remain the same */
	if(size == 3) return va_arg(*args, long);
	if(size == 4) return va_arg(*args, long long);
	return va_arg(*args, int);
}

static uint64_t load_u(int size, va_list *args)
{
	/* Again, no need to care about small types */
	if(size == 3) return va_arg(*args, unsigned long);
	if(size == 4) return va_arg(*args, unsigned long long);
	return va_arg(*args, unsigned int);
}

//---
//	Individual formatters
//---

/* kformat_char(): Character formatter (%c)
    (-)   Move spaces to the right
   {len}  Specifies numbers of (whitespace-padded) characters to print */
static void kformat_char(KFORMAT_ARGS)
{
	int c = va_arg(*args, int);

	struct geometry g = {
		.prefix = 0, .sign = 0, .content = 1,
	};
	kformat_geometry(spec, opt, &g);

	kprint_outn(' ', g.left_spaces);
	kprint_out(c);
	kprint_outn(' ', g.right_spaces);
}

/* kformat_str(): String formatter (%s)
    (-)   Move spaces to the right
   {len}  Minimal numbers of characters to output
   {pre}  Maximal numbers of bytes to read from argument */
static void kformat_str(KFORMAT_ARGS)
{
	char const *s = va_arg(*args, char const *);

	/* Compute content length, which is the smallest of two sizes: the
	   length set as precision and the actual length of the string */
	size_t len = 0;
	uint32_t precision = opt->precision ? opt->precision : (-1);
	while(s[len] && len < precision) len++;

	struct geometry g = {
		.prefix = 0, .sign = 0, .content = len,
	};
	kformat_geometry(spec, opt, &g);

	kprint_outn(' ', g.left_spaces);
	for(size_t i = 0; i < len; i++) kprint_out(s[i]);
	kprint_outn(' ', g.right_spaces);
}

/* kformat_int(): Integer formatter (%d, %i)
    (0)   Pad with zeros, rather than spaces, on the left
    (-)   Move spaces to the right (overrides '0', extends {pre})
    ( )   Force a blank sign before nonnegative numbers
    (+)   Force a sign before every number (overrides ' ')
   {len}  Minimal number of characters to print
   {pre}  Forces a minimal number of digits, creating 0s (overrides '0') */
static void kformat_int(KFORMAT_ARGS)
{
	int64_t n = load_i(opt->size, args);

	/* Compute the sign and the absolute value */
	struct geometry g = {
		.sign   = (n < 0) ? '-' : '+',
		.prefix = 0,
	};
	if(n < 0) n = -n;

	/* Get the digit string */
	char digits[32];
	int pure, total;

	pure = digits_10(digits, n);
	total = max(pure, opt->precision);
	g.content = total;

	kformat_geometry(spec, opt, &g);

	/* Print the result */
	kprint_outn(' ', g.left_spaces);
	if(g.sign) kprint_out(g.sign);
	kprint_outn('0', g.zeros);

	kprint_outn('0', total - pure);
	for(int i = pure - 1; i >= 0; i--) kprint_out(digits[i]);

	kprint_outn(' ', g.right_spaces);
}

/* kformat_uint(): Unsigned integer formatter in various bases (%u, %o, %x)
    (#)   Enable base prefixes ("0" in octal, "0x" in hexadecimal)
    (0)   Pad with zeros, rather than spaces, on the left
    (-)   Move spaces to the right (overrides '0', extends {pre})
   {len}  Minimal number of characters to print
   {pre}  Forces a minimal number of digits, creating 0s (overrides '0') */
static void kformat_uint(KFORMAT_ARGS)
{
	uint64_t n = load_u(opt->size, args);

	char digits[48];
	int pure = 0, total;
	int specl = spec | 0x20;

	if(specl == 'u') pure = digits_10(digits, n);
	if(specl == 'o') pure = digits_8(digits, n);
	if(specl == 'x') pure = digits_16(digits, spec == 'X', n);

	total = max(pure, opt->precision);

	/* Prefix length */
	size_t prefix = 0;
	if(opt->alternative) prefix = (specl != 'u') + (specl == 'x');

	struct geometry g = {
		.sign = 0, .prefix = prefix, .content = total
	};
	kformat_geometry(spec, opt, &g);

	/* Output */
	kprint_outn(' ', g.left_spaces);
	if(opt->alternative)
	{
		if(specl != 'u') kprint_out('0');
		if(specl == 'x') kprint_out(spec);
	}
	kprint_outn('0', g.zeros);

	kprint_outn('0', total - pure);
	for(int i = pure - 1; i >= 0; i--) kprint_out(digits[i]);
	kprint_outn(' ', g.right_spaces);
}

/* kformat_ptr(): Pointer formatter */
static void kformat_ptr(KFORMAT_ARGS)
{
	void *p = va_arg(*args, void *);

	char digits[] = "00000000";
	digits_16(digits, 0, (uint32_t)p);

	kprint_out('0');
	kprint_out('x');
	for(int i = 7; i >= 0; i--) kprint_out(digits[i]);
}

/* kformat_fixed(): Fixed-point decimal formatter
    (0)   Pad with zeros, rather than spaces, on the left
    (-)   Move spaces to the right (overrides '0')
    ( )   Force a blank sign before nonnegative numbers
    (+)   Force a sign before every number (overrides ' ')
   {len}  Minimal number of characters to print
   {pre}  Number of digits after the decimal dot */
static void kformat_fixed(KFORMAT_ARGS)
{
	int64_t n = load_i(opt->size, args);

	/* Compute the sign and the absolute value */
	struct geometry g = {
		.sign   = (n < 0) ? '-' : '+',
		.prefix = 0,
	};
	if(n < 0) n = -n;

	/* Get the digit string */
	char digits[32];

	g.content = digits_10(digits, n) + 1;
	kformat_geometry(spec, opt, &g);

	/* Print the result */
	kprint_outn(' ', g.left_spaces);
	if(g.sign) kprint_out(g.sign);
	kprint_outn('0', g.zeros);

	for(int i = g.content - 2; i >= 0; i--)
	{
		if(i == opt->precision - 1) kprint_out('.');
		kprint_out(digits[i]);
	}

	kprint_outn(' ', g.right_spaces);
}

//---
//	Standard formatted printing functions
//---

/* sprintf() */
GWEAK int sprintf(char *str, char const *format, ...)
{
	va_list args;
	va_start(args, format);

	int count = kvsprint(str, 65536, format, &args);

	va_end(args);
	return count;
}

/* vsprintf() */
GWEAK int vsprintf(char *str, char const *format, va_list args)
{
	return kvsprint(str, 65536, format, &args);
}

/* snprintf() */
GWEAK int snprintf(char *str, size_t n, char const *format, ...)
{
	va_list args;
	va_start(args, format);

	int count = kvsprint(str, n, format, &args);

	va_end(args);
	return count;
}

/* vsprintf() */
GWEAK int vsnprintf(char *str, size_t n, char const *format, va_list args)
{
	return kvsprint(str, n, format, &args);
}
