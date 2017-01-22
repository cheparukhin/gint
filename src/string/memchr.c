#include <string.h>
#include <stdint.h>

/*
	memchr()	O(byte_count)
	Looks for a byte in a memory area. Returns the address of the first
	occurrence if found, NULL otherwise.
*/
void *memchr(const void *area, int byte, size_t byte_count)
{
	const uint8_t *mem = area;

	while(byte_count--)
	{
		if(*mem == byte) return (void *)mem;
		mem++;
	}

	return NULL;
}
