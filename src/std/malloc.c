//---
// gint:std:malloc - Standard memory allocation functions
//---

#include <gint/kmalloc.h>
#include <gint/std/string.h>

/* malloc(): Allocate dynamic memory */
void *malloc(size_t size)
{
	return kmalloc(size, NULL);
}

/* free(): Free dynamic memory */
void free(void *ptr)
{
	kfree(ptr);
}

/* calloc(): Allocate and initialize dynamic memory */
void *calloc(size_t nmemb, size_t size)
{
	uint64_t total = (uint64_t)nmemb * (uint64_t)size;
	if(total >= 1ull << 32) return NULL;
	size = total;

	void *ptr = malloc(size);
	if(ptr) memset(ptr, 0, size);
	return ptr;
}

/* realloc(): Reallocate dynamic memory */
void *realloc(void *ptr, size_t size)
{
	return krealloc(ptr, size);
}
