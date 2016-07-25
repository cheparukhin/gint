#include <string.h>

/*
	strlen()
	Returns the length of a string.
*/
size_t strlen(const char *str)
{
	int len = 0;
	while(str[len]) len++;
	return len;
}
