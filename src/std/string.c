//---
//	gint:core:string - replicas of a few string functions
//---

#include <gint/defs/types.h>
#include <gint/defs/attributes.h>
#include <stdarg.h>

GWEAK size_t strlen(const char *str)
{
	int len = 0;
	while(str[len]) len++;
	return len;
}

/* vsprintf() - a trimmed-down version of the function
   This function supports formats '%%', '%nd', '%nx' and '%s' where 'n' is a
   1-digit size, and is mandatory. For '%d' and '%x', '0' is always set.
   Always outputs exactly the requested number of characters, even if it's not
   enough to completely print the value.
   Does whatever it wants if the format is invalid. This is really a basic
   function to format output without needing 18 kB of code. */
GWEAK void vsprintf(char *str, const char *format, va_list args)
{
	#define in() (c = *format++)

	const char *digits = "0123456789abcdef";
	int c, len;

	while(in())
	{
		if(c != '%')
		{
			*str++ = c;
			continue;
		}
		in();

		/* Length indications (only one character, not '%12d') */
		if(c >= '0' && c <= '9') len = c - '0', in();
		else len = -1;

		if(c == '%')
		{
			*str++ = '%';
		}
		else if(c == 'd')
		{
			int n = va_arg(args, int);
			if(n < 0) *str++ = '-', n = -n, len--;

			for(int i = len - 1; i >= 0; i--)
			{
				int m = n / 10;
				str[i] = digits[n - 10 * m];
				n = m;
			}
			str += len;
		}
		else if(c == 'x')
		{
			uint32_t n = va_arg(args, uint32_t);

			for(int i = len - 1; i >= 0; i--)
			{
				str[i] = digits[n & 0xf];
				n >>= 4;
			}
			str += len;
		}
		else if(c == 'c')
		{
			int c = va_arg(args, int);
			*str++ = c;
		}
		else if(c == 's')
		{
			const char *s = va_arg(args, const char *);
			while(*s && len) *str++ = *s++, len--;
		}
	}

	*str = 0;

	#undef in
	#undef out
}

/* sprintf() */
GWEAK void sprintf(char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
}
