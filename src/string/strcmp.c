#include <string.h>

/*
	strcmp()	O(max(len(str1), len(str2)))
	Compares two strings. Returns 0 if they are identical, a negative
	number if the first unequal byte is lower in str1 than in str2, and a
	positive number otherwise.
*/
int strcmp(const char *str1, const char *str2)
{
	while(*str1 && *str2 && *str1 == *str2)
	{
		str1++;
		str2++;
	}

	return *str1 - *str2;
}
