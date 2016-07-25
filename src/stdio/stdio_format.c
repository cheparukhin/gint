#include <internals/stdio.h>
#include <stdio.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/*
	Internal buffer.

	Using a buffer *really* simplifies everything. But it also has
	disadvantages, such a memory loss and limited output size.

	So, in case we find a possibility to get rid of this buffer, we will
	just have to change function character(), which is for now the only
	function that directly accesses the buffer.
*/
char __stdio_buffer[__stdio_buffer_size];



/*
	Format composed types.

	Format structure handles everything in a format, from data type to
	given value, including alternative forms, alignment and digit numbers.

	A format is made of a data type, which can be altered by a size option,
	a number of integer and decimal digits, and additional flags.

	The FormatFlags enumeration handles the various flags that can be added
	to a printf()-family format.
*/

enum FormatFlags
{
	// Option '#' specifies alternatives forms, mainly '0' and '0x'
	// prefixes in integer display.
	Alternative	= 1,
	// Under specific conditions, zero-padding may be used instead of
	// whitespace-padding.
	ZeroPadded	= 2,
	// Left alignment specifies that additional spaces should be added
	// after the value.
	LeftAlign	= 4,
	// In numeric display, this forces a blank sign to be written before
	// positive values.
	BlankSign	= 8,
	// In numeric display, this forces an explicit sign in all cases. This
	// option overrides BlankSign (see __printf() description for further
	// information on option precedence and influence).
	ForceSign	= 16
};

struct Format
{
	// Format type, one of 'diouxXcs' ('eEfFgGaApnm' still to add).
	char type;
	// Format size, one of 'l', 'h', 'i' ('i' means 'hh').
	char size;

	// Number of characters printed.
	int characters;
	// Number of digits after the dot.
	int precision;

	// Various flags.
	enum FormatFlags flags;

	// Value to output.
	union	
	{
		// Signed int with formats %c, %d and %i.
		signed int _int;
		// Insigned int with formats %o, %u, %x and %X.
		unsigned int _unsigned;
		// Double with formats %f, %F, %e, %E, %g and %G.
//		double _double;
		// String pointer with format %s.
		const char *_pointer;
	};
};



//---
//	Static declarations.
//---

// Outputs a character in the buffer. Updates counters.
static void character(int c);
// Outputs n timers the given character.
static void character_n(int c, int n);
// Reads a format from the given string pointer address (must begin with '%').
static struct Format get_format(const char **pointer);
// Computes the number of spaces and zeros to add to the bare output.
static void get_spacing(struct Format format, int *begin_spaces, int *sign,
	int *zeros, int length, int *end_spaces);

static void format_di	(struct Format format);
static void format_u	(struct Format format);
static void format_oxX	(struct Format format);
// static void format_e	(struct Format format);
static void format_c	(struct Format format);
static void format_s	(struct Format format);
static void format_p	(struct Format format);

#define abs(x) ((x) < 0 ? -(x) : (x))

// Number of characters currently written.
static size_t written = 0;
// Total number of function calls (characters theoretically written).
static size_t total = 0;
// Maximum number of characters to output.
static size_t max = 0;



/*
	character()
	Outputs a character to the buffer. This function centralizes all the
	buffer interface, so that if we come to remove it for property reasons,
	we would just have to edit this function.

	Static variables written and total are both needed, because the
	terminating NUL character may be written after the maximum has been
	reached.
	In other words, when the function ends, we need to have a variable
	counting the current position in the buffer (written), and one other
	containing the total number of theoretic characters (total) because
	these two values may be different.

	Of course the three variables need to be initialized before using this
	function.
*/
static void character(int c)
{
	// If there is space left in the buffer.
	if(written < max - 1) __stdio_buffer[written++] = c;
	total++;
}

/*
	character_n()
	Outputs n times the same character. Thought to be used to output spaces
	or zeros without heavy loops.
*/
static void character_n(int c, int n)
{
	int i = 0;
	while(i++ < n) character(c);
}

/*
	get_format()
	Reads the format from the given string pointer and returns a
	corresponding Format structure. The string pointer points to is also
	modified, so that is points to the first character after the format.
	This function expects **pointer == '%'.
*/
static struct Format get_format(const char **pointer)
{
	const char *convspec = "diouxXeEfFgGaAcspnm";
	struct Format format;

	const char *string = *pointer, *ptr;
	int c, i;
	char precision[10];

	// Moving the string pointer after the '%' character.
	string++;

	// Initializing structure.
	format.type	= 0;
	format.size	= 0;
	format.flags	= 0;
	// Initializing digit counts.
	format.characters = -1;
	format.precision = -1;

	// Parsing the format string. Testing each character until a
	// conversion specifier is found.
	while((c = *string))
	{
		// Looking for a conversion specifier.
		ptr = strchr(convspec, c);
		if(ptr)
		{
			format.type = *ptr;
			break;
		}

		// Looking for a left precision string (number of digits before
		// the dot), introduced by a non-null digit.
		if(c >= '1' && c <= '9')
		{
			format.characters = 0;
			for(i = 0; i < 9 && isdigit(*string); string++)
			{
				format.characters *= 10;
				format.characters += *string - '0';
			}

			// As pointer is now pointing to the next character,
			// we want to try tests again from the beginning.
			continue;
		}

		// Looking for a right precision string (number of digits after
		// the dot), introduced by a point.
		if(c == '.')
		{
			string++;
			if(!isdigit(*string)) continue;

			format.precision = 0;
			for(i = 0; i < 9 && isdigit(*string); string++)
			{
				format.precision *= 10;
				format.precision += *string - '0';
			}

			// As pointer is now pointing on the next character,
			// we want to try tests again from the beginning.
			continue;
		}

		// Handling predefined characters.
		switch(*string)
		{
		// Length modifiers.
		case 'h':
			format.size = 'h' + (format.size == 'h');
			break;
		case 'l':
		case 'L':
		case 'z':
		case 't':
			format.size = *string;
			break;

		// Flags.
		case '#':
			format.flags |= Alternative;
			break;
		case '0':
			format.flags |= ZeroPadded;
			break;
		case '-':
			format.flags |= LeftAlign;
			break;
		case ' ':
			format.flags |= BlankSign;
			break;
		case '+':
			format.flags |= ForceSign;
			break;
		}

		string++;
	}

	// If the format hasn't ended, the type attribute is left to zero and
	// the main loop will handle failure and break. Nothing has to be done
	// here.

	*pointer = string + 1;
	return format;
}

/*
	get_spacing()
	Computes the arrangement of beginning spaces, sign, zeros, pure value
	and ending spaces in formats.
	This formatting follows a recurrent model which is centralized in this
	function. Note that you can't have `begin_spaces` and `end_spaces`
	both non-zero: at least one is null.
*/
static void get_spacing(struct Format format, int *begin_spaces, int *sign,
	int *zeros, int length, int *end_spaces)
{
	// Using a list of types involving a sign.
	const char *signed_types = "dieEfFgGaA";
	int spaces;
	// Digits represents pure output + zeros (don't mix up with the *real*
	// displayed digits).
	int digits;
	int left = format.flags & LeftAlign;

	// Getting the total number of digits.
	switch(format.type)
	{
	// In integer display, the number of digits output is specified in the
	// precision.
	case 'd':
	case 'i':
	case 'u':
		digits = format.precision;
		if(digits < length) digits = length;
		break;

	// Binary display has prefixes such as '0' and '0x'.
	case 'o':
	case 'x':
	case 'X':
		digits = format.precision;
		if(digits == -1) digits = length;

		if(format.flags & Alternative)
		{
			int hexa = (format.type == 'x' || format.type == 'X');
			digits += 1 + hexa;
			length += 1 + hexa;
		}
		if(digits < length) digits = length;
		break;

	// Other formats do not have additional zeros.
	default:
		digits = length;
		break;
	}

	if(sign)
	{
		if(strchr(signed_types, format.type))
		{
			if(format.flags & BlankSign) *sign = ' ';
			// Option '+' overrides option ' '.
			if(format.flags & ForceSign) *sign = '+';
			// And of course negative sign overrides everything!
			if(format.type == 'd' || format.type == 'i')
			{
				if(format._int < 0) *sign = '-';
			}
//			else if(format._double < 0) *sign = '-';

		}
		else *sign = 0;
	}

	// Computing the number of spaces.
	spaces = format.characters - digits;
	// Computing the number of zeros.
	*zeros = digits - length;

	// Removing a space when a sign appears.
	if(sign && *sign) spaces--;

	// Option '0' translates spaces to zeros, but only if no precision is
	// specified; additionally, left alignment overrides zero-padding.
	if(!left && format.precision == -1 && format.flags & ZeroPadded)
	{
		*zeros += spaces;
		spaces = 0;
	}

	// Setting the correct spaces number to the computed value, depending
	// on the left alignment parameter.
	*begin_spaces	= left ? 0 : spaces;
	*end_spaces	= left ? spaces : 0;
}

/*
	__printf()

	Basic buffered formatted printing function. Fully-featured, so that
	ant call to a printf()-family function can be translated into a
	__printf() call.

	It always returns the number of characters of the theoretic formatted
	output. The real output may be limited in size by the given size
	argument when working with nprintf()-family functions, or the internal
	buffer itself.

	The Flags structure isn't necessary, but it simplifies a lot format
	argument handling (because flag effects depend on format type, which
	is unknown when the flags are read). Also, length modifiers 'hh' is
	stored as 'i' to simplify structure handling. 'll' is not supported.
	Format '%LF' is allowed by C99 and therefore supported.

	Generic information on options precedence and influence.
	-	Influences of integer part and mantissa digit numbers depend on
		the type of data that is being displayed.
	-	Option '#' doesn't contend with any other.
	-	Option '+' overrides options ' '.
	-	In integer display, option '0' translates spaces to zeros, but
		only if no decimal digit number is specified.
		The option '-' also overrides it, forcing whitespaces to be
		written at the end of the format.

	Limit of function.
	-	Internal buffer size (should be customizable with a -D option
		when compiling).
	-	Precision values (format %a.b) are written on 8 bits, therefore
		limited to 127.

	Unsupported features.
	-	Flag character ''' (single quote) for thousands grouping
	-	Flag character 'I', that outputs locale's digits (glibc 2.2)
	-	Length modifiers 'll' and 'q' (libc 5 and 4.4 BSD)
	-	This is not really a feature but incorrect characters in
		formats are ignored and don't invalidate the format.
*/
int __printf(size_t size, const char *string, va_list args)
{
	struct Format format;

	// Avoiding overflow by adjusting the size argument.
	if(!size || size > __stdio_buffer_size)
		size = __stdio_buffer_size;

	// Initializing character() working values.
	written	= 0;
	total	= 0;
	max	= size;

	// Parsing the format string. At each iteration, a literal character, a
	// '%%' identifier or a format is parsed.
	while(*string)
	{
		// Literal text.
		if(*string != '%')
		{
			character(*string++);
			continue;
		}

		// Literal '%'.
		if(string[1] == '%')
		{
			string += 2;
			character('%');
			continue;
		}

		// Getting the format.
		format = get_format(&string);
		if(!format.type) break;

/*
		// Displaying an information message.
		printf(
			"Format found :%s%c%c, options %d, and %d.%d "
			"digits\n",
			format.size ? " " : "",
			format.size ? format.size : ' ',
			format.type,
			format.flags,
			format.digits,
			format.mantissa
		);
*/

		switch(format.type)
		{
		// Signed integers.
		case 'd':
		case 'i':
			format._int = va_arg(args, signed int);

			// Reducing value depending on format size.
			switch(format.size)
			{
			case 'h':
				format._int &= 0x0000ffff;
				break;
			case 'i':
				format._int &= 0x000000ff;
				break;
			}

			format_di(format);
			break;

		// Unsigned integers.
		case 'u':
			format._unsigned = va_arg(args, unsigned int);
			format_u(format);
			break;
		case 'o':
		case 'x':
		case 'X':
			format._unsigned = va_arg(args, unsigned int);
			format_oxX(format);
			break;

/*		// Exponent notation.
		case 'e':
		case 'E':
			format._double = va_arg(args, double);
			format_e(format);
			break;
*/
		// Characters.
		case 'c':
			format._int = va_arg(args, signed int) & 0xff;
			format_c(format);
			break;

		// Strings.
		case 's':
			format._pointer = va_arg(args, const char *);
			format_s(format);
			break;

		// Pointers.
		case 'p':
			format._unsigned = va_arg(args, unsigned int);
			format_p(format);
			break;

		// Character counter.
		case 'n':
			*va_arg(args, int *) = written;
			break;
		}
	}

	// Adding a terminating NUL character. Function character() should have
	// left an empty byte for that.
	__stdio_buffer[written] = 0;
	return total;
}

/*
	format_di()

	Subroutine itoa(). Writes the given signed integer to the internal
	buffer, trough function character().
	It is used by conversion specifiers 'd' and 'i'.
	Options '#' and '0' have no effect.
*/
static void format_di(struct Format format)
{
	// In integer display, character number includes pure digits and
	// additional zeros and spacing.
	// The precision represents the number of digits (pure digits and
	// zeros) to print.
	// For example: ' 0004', pure digits: 1, digits: 4, characters: 5.

	int sign = 0;
	signed int x = format._int;
	// Using integers to store the number pure digits and additional spaces
	// and zeros.
	int bspaces, zeros, digits = 0, espaces;
	// Using a multiplier to output digit in the correct order.
	int multiplier = 1;

	// Returning if the argument is null with an explicit precision of
	// zero, but only if there are no spaces.
	if(!x && format.characters == -1 && !format.precision) return;



	//---
	//	Computations.
	//---

	// Computing the number of digits and the multiplier.
	x = abs(format._int);
	if(!x) digits = 1;
	else while(x)
	{
		digits++;
		x /= 10;
		if(x) multiplier *= 10;
	}

	// Getting the corresponding spacing.
	get_spacing(format, &bspaces, &sign, &zeros, digits, &espaces);



	//---
	//	Output.
	//---

	character_n(' ', bspaces);
	if(sign) character(sign);
	character_n('0', zeros);

	x = abs(format._int);
	// Writing the pure digits, except if the value is null with an
	// explicit precision of zero.
	if(x || format.precision) while(multiplier)
	{
		character((x / multiplier) % 10 + '0');
		multiplier /= 10;
	}

	character_n(' ', espaces);
}

/*
	format_u()
	Unsigned integers in base 10. Options ' ', '+' and '#' have no effect.
*/
static void format_u(struct Format format)
{
	int bspaces, zeros, digits = 0, espaces;
	int x = format._unsigned;
	int multiplier = 1;
	int c;

	// Computing number of digits.
	if(!x) digits = 1;
	else while(x)
	{
		digits++;
		x /= 10;
		if(x) multiplier *= 10;
	}

	get_spacing(format, &bspaces, NULL, &zeros, digits, &espaces);

	//---
	//	Output.
	//---

	character_n(' ', bspaces);
	character_n('0', zeros);

	x = format._unsigned;
	while(multiplier)
	{
		character('0' + (x / multiplier) % 10);
		multiplier /= 10;
	}

	character_n(' ', espaces);
}

/*
	format_oxX()
	Unsigned integers in base 8 or 16.
	Since the argument is unsigned, options ' ' and '+' have no effect.
	Option '#' adds prefix '0' in octal or '0x' in hexadecimal.
*/
static void format_oxX(struct Format format)
{
	// In unsigned display, the digit number specifies the minimal number
	// of characters that should be output. If the prefix (alternative
	// form) is present, it is part of this count.
	// Integer part and decimal part digit numbers behave the same way as
	// in signed integer display.

	// Using integers to store the number of digits, zeros and spaces.
	int bspaces, zeros, digits = 0, espaces;
	unsigned int x = format._unsigned;
	int multiplier = 0, shift, mask;
	int c, disp;



	//---
	//	Computations.
	//---

	shift	= (format.type == 'o') ? (3) : (4);
	mask 	= (1 << shift) - 1;
	disp	= (format.type == 'x') ? (39) : (7);

	// Computing number of digits.
	if(!x) digits = 1;
	else while(x)
	{	
		digits++;
		x >>= shift;
		if(x) multiplier += shift;
	}

	// Getting the spacing distribution.
	get_spacing(format, &bspaces, NULL, &zeros, digits, &espaces);



	//---
	//	Output.
	//---

	character_n(' ', bspaces);
	x = format._unsigned;

	// Writing the alternative form prefix.
	if(format.flags & Alternative && x)
	{
		character('0');
		if(format.type != 'o') character(format.type);
	}

	character_n('0', zeros);

	// Extracting the digits.
	while(multiplier >= 0)
	{
		c = (x >> multiplier) & mask;
		c += '0' + (c > 9) * disp;

		character(c);
		multiplier -= shift;
	}

	character_n(' ', espaces);
}

/*
	format_e()
	Exponent notation. Option '#' has no effect.

static void format_e(struct Format format)
{
	// In exponent display, the precision is the number of digits after the
	// dot.

	// Using an integer to store the number exponent.
	int exponent = 0;
	// Using a double value for temporary computations, and another to
	// store the format parameter.
	double tmp = 1, x = format._double;
	// Using spacing variables. Default length is for '0.e+00';
	int bspaces, zeros, sign, length = 6, espaces;
	// Using an iterator and a multiplier.
	int i, mul;



	//---
	//	Computations.
	//---

	// Computing the exponent. For positive exponents, increasing until
	// the temporary value gets greater than x.
	if(x > 1)
	{
		// Looping until we haven't reached a greater exponent.
		while(tmp < x)
		{
			// Incrementing the exponent.
			exponent++;
			// Multiplying the test value.
			tmp *= 10;
		}
		// Removing an additional incrementation.
		exponent--;
	}
	// For negative exponents, decreasing until it's lower.
	else while(tmp > x)
	{
		// Decrementing the exponent.
		exponent--;
		// Dividing the test value.
		tmp *= 0.1;
	}

	// Adding a character if the exponent is greater that 100.
	if(exponent >= 100) length++;
	// Adding another one if it's greater than 1000.
	if(exponent >= 1000) length++;

	// Adjusting the format precision, defaulting to 6.
	if(format.precision == -1) format.precision = 6;
	// Adding the decimal digits.
	length += format.precision;

	// Getting the space repartition.
	get_spacing(format, &bspaces, &sign, &zeros, length, &espaces);



	//---
	//	Output.
	//---

	// Writing the beginning whitespaces.
	character_n(' ', bspaces);
	// Writing the sign if existing.
	if(sign) character(sign);
	// Writing the zeros.
	character_n('0', zeros);

	// Initializing x.
	x = abs(format._double) / tmp;
	// Writing the first digit.
	character(x + '0');
	character('.');

	// Writing the decimal digits.
	for(i = 0; i < format.precision; i++)
	{
		// Multiplying x by 10 and getting rid of the previous digit.
		x = (x - (int)x) * 10;
		// Writing the current digit.
		character(x + '0');
	}

	// Writing the exponent letter and its sign.
	character(format.type);
	character(exponent < 0 ? '-' : '+');

	// Getting a positive exponent.
	exponent = abs(exponent);

	// Using a multiplier for the exponent.
	if(exponent >= 1000) mul = 1000;
	else if(exponent >= 100) mul = 100;
	else mul = 10;

	// Writing the exponent characters.
	while(mul)
	{
		// Writing the next character.
		character((exponent / mul) % 10 + '0');
		// Dividing the multiplier.
		mul *= 0.1;
	}

	// Writing the ending whitespaces if left-aligned.
	character_n(' ', espaces);
}
*/

/*
	format_c()
	Character output. Only handles left alignment and spacing.
	Options '#', '0', ' ' and '+', as well as mantissa digit number, have
	no effect.
*/
static void format_c(struct Format format)
{
	// In character display, the digit number represents the number of
	// characters written, including the argument and additional
	// whitespaces.

	int spaces = format.characters - 1;
	int left = format.flags & LeftAlign;

	if(!left) character_n(' ', spaces);
	character(format._int & 0xff);
	if(left) character_n(' ', spaces);
}

/*
	format_s()
	String output. Spaces if needed.
*/
void format_s(struct Format format)
{
	// In string display, the character number specify the minimum size of
	// output (padded with whitespaces if needed) and the precision
	// specify the maximum number of string characters output.

	int string = format.precision;
	int spaces;

	const char *str = format._pointer;
	int length, i;
	int left = format.flags & LeftAlign;

	// Computing length of string and number of whitespaces.
	length = strlen(str);
	if(string > length || string == -1) string = length;
	spaces = format.characters - string;

	if(!left) character_n(' ', spaces);
	for(i = 0; i < string; i++) character(str[i]);
	if(left) character_n(' ', spaces);
}

/*
	format_p()
	Pointer output. Simple hexadecimal dump. Prints "(nil)" if pointer is
	NULL.
*/
void format_p(struct Format format)
{
	// Pointer display falls back to %#08x in the pointer is non-null,
	// "(nil)" otherwise.

	unsigned int x = format._unsigned;
	int bspaces, zeros, digits = 0, espaces;
	int c;

	digits = x ? 10 : 5;
	get_spacing(format, &bspaces, NULL, &zeros, digits, &espaces);

	character_n(' ', bspaces);
	character_n('0', zeros);

	if(x)
	{
		character('0');
		character('x');
		while(x)
		{
			c = x >> 28;
			c += '0' + 39 * (c > 9);
			character(c);
			x <<= 4;
		}
	}
	else
	{
		character('(');
		character('n');
		character('i');
		character('l');
		character(')');
	}

	character_n(' ', espaces);
}
