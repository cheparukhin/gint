#include <string.h>

/*
	strncpy()
	Copies part of a string to another.
*/
char *strncpy(char *destination, const char *source, size_t size)
{
	size_t length = strlen(source);

	if(length >= size)
	{
		return memcpy(destination, source, size);
	}
	else
	{
		memset(destination + length, 0, size - length);
		return memcpy(destination, source, length);
	}
}
