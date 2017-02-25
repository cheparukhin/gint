//---
//	vsprintf()
//
//	Unfortunately this function, which is part of fxlib, was broken by a
//	terribly un-professional port (incidentally, I am responsible for
//	this, so I can't complain). So we'll need something simple...
//
//      Format    Flags     Character count
//	  %d        none       no
//        %x        0          yes
//        %p        none       no
//        %c        none       no
//        %s        none       no
//---

#include <stdarg.h>
#include <stdint.h>

void vsprintf_int(char **buffer_ptr, int n)
{
	char *buffer = *buffer_ptr;
	if(!n)
	{
		*buffer++ = '0';
		*buffer_ptr = buffer;
		return;
	}
	if(n < 0)
	{
		*buffer++ = '-';
		n = -n;
	}

	int digits = 0, x = n, copy;
	while(x) digits++, x /= 10;
	copy = digits;

	while(digits)
	{
		buffer[--digits] = n % 10 + '0';
		n /= 10;
	}

	*buffer_ptr = buffer + copy;
}

void vsprintf_hexa(char **buffer_ptr, uint32_t val, int digits, int zero)
{
	char *buffer = *buffer_ptr;
	if(!val)
	{
		while(digits-- > 1) *buffer++ = (zero) ? '0' : ' ';
		*buffer++ = '0';
		*buffer_ptr = buffer;
		return;
	}

	if(digits <= 0)
	{
		uint32_t x = val;
		while(x) digits++, x >>= 4;
	}
	int copy = digits;

	while(val && digits)
	{
		buffer[--digits] = (val & 15) + '0' + 39 * ((val & 15) > 9);
		val >>= 4;
	}
	while(digits)
	{
		buffer[--digits] = (zero) ? '0': ' ';
	}

	*buffer_ptr = buffer + copy;
}

void vsprintf_ptr(char **buffer_ptr, void *ptr)
{
	vsprintf_hexa(buffer_ptr, (uint32_t)ptr, 8, 1);
}

void vsprintf_char(char **buffer_ptr, int c)
{
	char *buffer = *buffer_ptr;
	*buffer++ = c;
	*buffer_ptr = buffer;
}

void vsprintf_str(char **buffer_ptr, const char *str)
{
	char *buffer = *buffer_ptr;
	while(*str) *buffer++ = *str++;
	*buffer_ptr = buffer;
}

int vsprintf(char *buffer, const char *format, va_list args)
{
	char *save = buffer;
	int zero, count;

	while(*format)
	{
		if(*format != '%')
		{
			*buffer++ = *format++;
			continue;
		}
		if(!*++format) break;

		zero = 0;
		count = 0;

		if(*format == '0') zero = 1, format++;
		while(*format >= '0' && *format <= '9')
		{
			count *= 10;
			count += (*format++ - '0');
		}
		if(!*format) break;

		switch(*format)
		{
		case 'd':
			vsprintf_int(&buffer, va_arg(args, int));
			break;
		case 'x':
			vsprintf_hexa(&buffer, va_arg(args, uint32_t), count,
				zero);
			break;
		case 'p':
			vsprintf_ptr(&buffer, va_arg(args, void *));
			break;
		case 'c':
			vsprintf_char(&buffer, va_arg(args, int));
			break;
		case 's':
			vsprintf_str(&buffer, va_arg(args, const char *));
			break;
		default:
			*buffer++ = *format;
			break;
		}

		format++;
	}

	*buffer = 0;
	return buffer - save;
}

int sprintf(char *buffer, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int x = vsprintf(buffer, format, args);
	va_end(args);

	return x;
}
