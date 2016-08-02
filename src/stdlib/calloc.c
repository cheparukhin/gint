#include <stdlib.h>
#include <string.h>

/*
	calloc()
	Allocates 'n' elements of size 'size' and wipes the memory area.
	Returns NULL on error.
*/
void *calloc(size_t n, size_t size)
{
	void *ptr = malloc(n * size);
	return ptr ? memset(ptr, 0, n * size) : NULL;
}
