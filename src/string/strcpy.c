#include <string.h>

/*
	strcpy()
	Copies a string to another.
*/
char *strcpy(char *destination, const char *source)
{
	size_t length = strlen(source);
	return memcpy(destination, source, length);
}
