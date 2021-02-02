//---
//	gint:kprint - gint's printf(3)-like formatted printer
//
//	Things to change when creating new formats:
//	-> List of prototypes at the end of the definition section
//	-> Formatted table just below the list of prototypes
//	-> List of non-numerical specifiers in kformat_geometry()
//---

#include <gint/std/stdio.h>
#include <gint/std/string.h>
#include <gint/kprint.h>

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>
#include <gint/defs/util.h>

#include <stdarg.h>

//---
//	kprint() definitions
//---

#define KPRINT_BUFSIZE 64

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

/* kprint_register(): Register a formatter */
void kprint_register(int spec, kprint_formatter_t kformat)
{
	/* Non-letters */
	if(spec < 'a' || spec > 'z') return;
	/* Size-specifying letters */
	if(spec == 'h' || spec == 'l' || spec == 'z') return;

	kprint_formatters[spec - 'a'] = kformat;
}

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

/* kprint_out(): Output a single character to the kprint buffer */
GINLINE void kprint_out(int byte)
{
	if(kprint_ptr >= kprint_limit) kprint_flush();
	*kprint_ptr++ = byte;
}

/* kprint_outn(): Output the same character <n> times */
GINLINE void kprint_outn(int byte, int n)
{
	while(n-- > 0) kprint_out(byte);
}

/* kprint_outstr(): Output a string to the kprint buffer */
GINLINE void kprint_outstr(char const *str, int n)
{
	for(int i = 0; i < n; i++) kprint_out(str[i]);
}

//---
//	Parsing helpers
//---

/* kprint_opt(): Parse option strings */
kprint_options_t kprint_opt(char const **options_ptr)
{
	/* No options enabled by default, set the size to int */
	kprint_options_t opt = { .size = 2, .precision = -1 };

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
			opt.precision = 0;
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
		if(c == 'z') opt.size = 3;

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
	kprint_options_t opt;

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
			format++;
			continue;
		}

		opt = kprint_opt(&format);
		spec = *format++;

		int id = (spec | 0x20) - 'a';
		if(id >= 0 && id < 26 && kprint_formatters[id])
			kprint_formatters[id](spec, &opt, args);
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

/* kformat_geometry(): Calculate the geometry of a format */
void kformat_geometry(kprint_options_t *opt, kprint_geometry_t *g)
{
	int integral = (g->style == KPRINT_INTEGER);
	int numerical = (g->style == KPRINT_NUMERIC) || integral;
	ssize_t padding;

	/* Sign character (no discussion required for negative values) */
	if(numerical && g->sign == '+')
	{
		g->sign = 0;
		if(opt->blank_sign) g->sign = ' ';
		if(opt->force_sign) g->sign = '+';
	}

	g->zeros = 0;

	padding = opt->length - (g->sign != 0) - g->prefix
		- max(g->content, opt->precision);
	if(padding < 0) padding = 0;

	/* In integral modes, precision forces zeros */
	if(integral && opt->precision >= 0)
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
	/* All smaller types are promoted to int with sign extension, so we
	   don't need to care about them. */
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

	kprint_geometry_t g = {
		.prefix = 0, .sign = 0, .content = 1,
	};
	kformat_geometry(opt, &g);

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

	kprint_geometry_t g = {
		.prefix = 0, .sign = 0, .content = len,
	};
	kformat_geometry(opt, &g);

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
	kprint_geometry_t g = {
		.sign   = (n < 0) ? '-' : '+',
		.prefix = 0,
		.style  = KPRINT_INTEGER,
	};
	if(n < 0) n = -n;

	/* Get the digit string */
	char digits[32];
	int pure, total;

	pure = digits_10(digits, n);
	if(opt->precision == 0 && !n) pure = 0;
	total = max(pure, opt->precision);
	g.content = total;

	kformat_geometry(opt, &g);

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

	if(opt->precision == 0 && !n) pure = 0;
	total = max(pure, opt->precision);

	/* Prefix length */
	size_t prefix = 0;
	if(opt->alternative) prefix = (specl != 'u') + (specl == 'x');

	kprint_geometry_t g = {
		.sign = 0, .prefix = prefix, .content = total,
		.style = KPRINT_INTEGER,
	};
	kformat_geometry(opt, &g);

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
	kprint_geometry_t g = {
		.sign   = (n < 0) ? '-' : '+',
		.prefix = 0,
		.style  = KPRINT_NUMERIC,
	};
	if(n < 0) n = -n;

	/* Get the digit string */
	char digits[32];

	g.content = digits_10(digits, n) + 1;
	kformat_geometry(opt, &g);

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
