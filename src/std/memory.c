#include <gint/defs/attributes.h>
#include <gint/hardware.h>
#include <stddef.h>
#include <stdint.h>

static void memcpy4(uint32_t * restrict d, const void * restrict src, size_t n)
{
	int modulo = (uintptr_t)src & 3;

	/* Best case: perform 32-bit accesses only */
	if(!modulo)
	{
		const uint32_t *s = src;
		for(; n; n-=4) *d++ = *s++;
	}

#if 0
	/* Here's where SH-3 and SH-4A start working differently. SH-4A has a
	   2-cycle 'movua' instruction to perform unaligned reads */
	else if(isSH4())
	{
		uint32_t longword;
		const uint32_t *s = src;

		while(n--)
		{
			__asm__(
				"movua.l %1, %0"
				: "=z"(longword)
				: "m>"(*s)
			);
			s++;
			*d++ = longword;
		}
	}
#endif

	/* On SH-3, we can only hope that there is 2-alignment */
	else if(!(modulo & 1))
	{
		const uint16_t *s = src;
		uint16_t * restrict dst = (void *)d;

		for(; n; n-=2)
		{
			*dst++ = *s++;
			*dst++ = *s++;
		}
	}

	/* Or just perform the raw copy */
	else
	{
		const uint8_t *s = src;
		uint8_t * restrict dst = (void *)d;

		while(n--) *dst++ = *s++;
	}
}

void *memcpy(void * restrict dst, const void * restrict src, size_t n)
{
	uint8_t *d = dst;
	const uint8_t *s = src;

	/* Small areas: don't bother with complex methods */
	if(n < 32)
	{
		while(n--) *d++ = *s++;
		return dst;
	}

	/* Find a longword offset to perform word or longword operations */
	while((uintptr_t)d & 3) *d++ = *s++, n--;

	/* Perform the big, efficient copy */
	memcpy4((void *)d, s, n & ~3);

	size_t m = n & 3;
	d += (n - m);
	s += (n - m);
	n = m;

	/* Copy around the last bytes */
	while(n--) *d++ = *s++;
	return dst;
}

void *_memmove(GUNUSED void *dst, GUNUSED const void *src, GUNUSED size_t n)
{
	// (same as memcpy, but heed for direction if areas overlap)

	// copy by increasing addresses if dst < src
	// copy by decreasing addresses if dst > src
	return dst;
}

int _memcmp(GUNUSED const void *s1, GUNUSED const void *s2, GUNUSED size_t n)
{
	return 0;
}

void *memset(void *s, int byte, size_t n)
{
	/* TODO: Do it efficiently */
	char *dst = s;
	while(n--) *dst++ = byte;
	return s;
}
