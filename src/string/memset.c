#include <string.h>
#include <stdint.h>

/*
	memset()
	Sets the contents of a memory area. A smart copy is performed.
*/
void *memset(void *d, int byte, size_t byte_number)
{
	uint8_t *dest = (uint8_t *)d;
	unsigned short word = (byte << 8) | byte;
	unsigned int longword = (word << 16) | word;

	// When the area is small, simply copying using byte operations. The
	// minimum length used for long operations must be at least 3.
	if(byte_number < 8)
	{
		while(byte_number)
		{
			*dest++ = byte;
			byte_number--;
		}

		return d;
	}

	// Reaching a long offset.
	while((intptr_t)dest & 3)
	{
		*dest++ = byte;
		byte_number--;
	}
	// Copying using long operations.
	while(byte_number >= 4)
	{
		*((uint32_t *)dest) = longword;
		dest += 4;
		byte_number -= 4;
	}
	// Ending the copy.
	while(byte_number)
	{
		*dest++ = byte;
		byte_number--;
	}

	return d;
}
