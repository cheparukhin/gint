#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifndef	GINT_NO_SYSCALLS
void __free(void *ptr);
#endif

/*
	free()
	Frees a memory block allocated by malloc(), calloc() or realloc().
*/
void free(void *ptr)
{
	#ifndef	GINT_NO_SYSCALLS
	__free(ptr);
	#endif
}

#pragma GCC diagnostic pop
