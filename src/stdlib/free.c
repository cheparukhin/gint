#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifndef	GINT_NO_SYSCALLS
#include <internals/syscalls.h>
#endif

/*
	free()
	Frees a memory block allocated by malloc(), calloc() or realloc().
*/
void free(void *ptr)
{
	// Just to be sure.
	if(!ptr) return;

	#ifndef	GINT_NO_SYSCALLS
	__free(ptr);
	#endif
}

#pragma GCC diagnostic pop
