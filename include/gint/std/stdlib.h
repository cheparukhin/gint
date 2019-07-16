//---
//	gint:std:stdlib - a few <stdlib.h> functions provided by gint
//---

#ifndef GINT_STD_STDLIB
#define GINT_STD_STDLIB

#include <stddef.h>

/* malloc(): Allocate dynamic memory */
void *malloc(size_t size);

/* free(): Free dynamic memory */
void free(void *ptr);

/* calloc(): Allocate and initialize dynamic memory */
void *calloc(size_t nmemb, size_t size);

/* realloc(): Reallocate dynamic memory */
void *realloc(void *ptr, size_t size);

#endif /* GINT_STD_STDLIB */
