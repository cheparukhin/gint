#include <stdlib.h>

/*
	malloc()
	Allocates 'size' bytes and returns a pointer to a free memory area.
	Returns NULL on error.
*/

#ifndef	GINT_NO_SYSCALLS
#include <internals/syscalls.h>

void *malloc(size_t size)
{
	return __malloc(size);
}

#else

#pragma GCC diagnostic ignored "-Wunused-parameter"
void *malloc(size_t size)
{
	return NULL;
}
#pragma GCC diagnostic pop

#endif

