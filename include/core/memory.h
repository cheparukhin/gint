//---
//	core:memory - Core memory functions
//
//	These are the basic standard memory functions required by GCC.
//---

#ifndef GINT_CORE_MEMORY
#define GINT_CORE_MEMORY

/* memcpy() - copy a chunk of memory to a non-overlapping destination */
void *memcpy(void * restrict dest, const void * restrict src, size_t n);

/* memmove() - copy a chunk of memory to any destination */
void *memmove(void *dest, const void *src, size_t n);

/* memcmp() - compare two chunks of memory of the same size */
int memcmp(const void *src1, const void *src2, size_t n);

/* memset() - fill a chunk of memory with a single byte */
void *memset(void *dest, int byte, size_t n);

#endif /* GINT_CORE_MEMORY */
