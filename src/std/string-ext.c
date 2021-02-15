//---
//	gint:std:string-ext: Extensions to the standard for <string.h>
//---

#include <gint/defs/attributes.h>
#include <gint/std/string.h>

GWEAK char *strchrnul(char const *s, int c)
{
	while(*s && *s != c) s++;
	return (char *)s;
}
