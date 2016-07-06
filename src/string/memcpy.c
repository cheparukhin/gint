#include <string.h>
#include <stdint.h>

/*
	memcpy()
	Copies a memory area. A smart copy is performed if possible.
*/
void *memcpy(void *d, const void *s, size_t n)
{
	uint8_t		*dest = (uint8_t *)d;
	const uint8_t	*src  = (const uint8_t *)s;

	// A long-based copy needs the source and destination to be 4-aligned
	// at the same time.
	if(((intptr_t)dest & 3) == ((intptr_t)src & 3))
	{
		// Getting to a long offset.
		while((intptr_t)dest & 3)
		{
			*dest++ = *src++;
			n--;
		}

		// Copying groups of four bytes.
		while(n >= 4)
		{
			*((uint32_t *)dest) = *((const uint32_t *)src);

			dest += 4, src += 4;
			n -= 4;
		}

		// Ending the copy.
		while(n)
		{
			*dest++ = *src++;
			n--;
		}
	}

	// Or we could try a word-based copy.
	else if(((intptr_t)dest & 1) == ((intptr_t)src & 1))
	{
		// Getting to a word offset.
		if((intptr_t)dest & 1)
		{
			*dest++ = *src++;
			n--;
		}

		// Copying groups of two bytes.
		while(n >= 2)
		{
			*((uint16_t *)dest) = *((const uint16_t *)src);

			dest += 2, src += 2;
			n -= 2;
		}

		// Ending the copy.
		while(n)
		{
			*dest++ = *src++;
			n--;
		}
	}

	// In some cases we can just perform a raw copy.
	else while(n)
	{
		*dest++ = *src++;
		n--;
	}

	return d;
}
