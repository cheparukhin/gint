#include <string.h>
#include <stdint.h>

int raw_cmp(const uint8_t *mem1, const uint8_t *mem2, size_t byte_count);

/*
	memcmp()	O(byte_count)
	Compares two memory areas. Returns 0 if all bytes are equal in both
	areas, a negative number if the first unequal byte is lower in the
	first area, and a positive number otherwise.
	A smart comparison is performed when possible.
*/
int memcmp(const void *area1, const void *area2, size_t byte_count)
{
	const uint8_t	*mem1 = (const uint8_t *)area1;
	const uint8_t	*mem2 = (const uint8_t *)area2;

	// Trying to do long-based comparisons.
	if(((intptr_t)area1 & 3) == ((intptr_t)area2 & 3))
	{
		// Getting to a 4-byte offset.
		while((intptr_t)mem1 & 3)
		{
			if(*mem1 != *mem2) return *mem1 - *mem2;
			mem1++, mem2++;
			byte_count--;
		}

		// Testing groups of four bytes.
		while(byte_count >= 4)
		{
			uint32_t long1 = *((uint32_t *)mem1);
			uint32_t long2 = *((uint32_t *)mem2);
			if(long1 != long2) return raw_cmp(mem1, mem2, 4);
			mem1 += 4, mem2 += 4;
			byte_count -= 4;
		}

		// Testing the last bytes.
		return raw_cmp(mem1, mem2, byte_count);
	}

	// Well, maybe we can do a word-based operation?
	else if(((intptr_t)area1 & 1) == ((intptr_t)area2 & 3))
	{
		// Getting to the word offset.
		if((intptr_t)mem1 & 1)
		{
			if(*mem1 != *mem2) return *mem1 - *mem2;
			mem1++, mem2++;
			byte_count--;
		}

		// Testing groups of two bytes.
		while(byte_count >= 2)
		{
			uint16_t word1 = *((uint16_t *)mem1);
			uint16_t word2 = *((uint16_t *)mem2);
			if(word1 != word2) return raw_cmp(mem1, mem2, 2);
			mem1 += 2, mem2 += 2;
			byte_count -= 2;
		}

		// Testing the last byte.
		if(!byte_count) return 0;
		return *mem1 - *mem2;
	}

	// That's too bad, we'll have to compare everything manually.
	else return raw_cmp(mem1, mem2, byte_count);
}

int raw_cmp(const uint8_t *mem1, const uint8_t *mem2, size_t byte_count)
{
	while(byte_count)
	{
		if(*mem1 != *mem2) return *mem1 - *mem2;
		mem1++;
		mem2++;
		byte_count--;
	}

	return 0;
}
