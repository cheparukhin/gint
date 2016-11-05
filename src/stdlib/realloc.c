#include <stdlib.h>

/*
	realloc()
	Reallocates a memory block and moves its data.
*/

#ifndef	GINT_NO_SYSCALLS

void *__realloc(void *ptr, size_t size);
void *realloc(void *ptr, size_t size)
{
	return __realloc(ptr, size);
}

#else

#pragma GCC diagnostic ignored "-Wunused-parameter"
void *realloc(void *ptr, size_t size)
{
	return NULL;
}
#pragma GCC diagnostic pop

#endif
