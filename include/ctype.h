#ifndef _CTYPE_H
#define _CTYPE_H 1

// Character definition macros.
#define isalnum(c)	(isdigit(c)	|| isalpha(c))
#define isalpha(c)	(islower(c)	|| isupper(c))

#define iscntrl(c)	((c) <= 0x1f	|| (c) == 0x7f)
#define isdigit(c)	((c) >= '0'	&& (c) <= '9')
#define isgraph(c)	((c) > ' '	&& (c) < 0x7f)
#define islower(c)	((c) >= 'a'	&& (c) <= 'z')
#define isprint(c)	((c) >= ' '	&& (c) < 0x7f)
#define ispunct(c)     (((c) >= '!'	&& (c) <= '/') || \
			((c) >= ':'	&& (c) <= '@') || \
			((c) >= '['	&& (c) <= '`') || \
			((c) >= '{'	&& (c) <= '~'))
#define isspace(c)     (((c) >= '\t'	&& (c) <= '\r') || (c) == ' ')
#define isupper(c)	((c) >= 'A'	&& (c) <= 'Z')
#define isxdigit(c)    (((c) >= '0'	&& (c) <= '9') || \
			((c) >= 'A'	&& (c) <= 'F') || \
			((c) >= 'a'	&& (c) <= 'f'))

// Character manipulation macros.
#define tolower(c) (isupper(c) ? (c) |  32 : (c))
#define toupper(c) (islower(c) ? (c) & ~32 : (c))

#endif // _CTYPE_H
