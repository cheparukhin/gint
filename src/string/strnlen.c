#include <string.h>

/*
	strnlen()	O(len(str))
	Returns the minimum of the length of the string and n. This function
	never access more than n bytes at the beginning of the string.
*/
size_t strnlen(const char *str, size_t n)
{
	size_t len = 0;
	while(len < n && str[len]) len++;
	return len;
}
