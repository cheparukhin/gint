#include <internals/tales.h>
#include <tales.h>

/*
	text_length()
	Computes the length of a string using the currently configured font.
*/
size_t text_length(const char *str)
{
	if(!str) return 0;
	size_t len = 0;

	while(*str)
	{
		int index = getCharacterIndex(*str);
		len += font->glyphs[index] >> 24;
		if(*++str) len++;
	}

	return len;
}
