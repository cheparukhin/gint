//---
// gint:kmalloc - gint's memory allocator
//---

#ifndef GINT_KMALLOC
#define GINT_KMALLOC

#include <gint/defs/types.h>

//---
// Standard memory allocation API
//---

/* kmalloc(): Allocate memory in one of the available arenas
   This function acts like malloc(). The second parameter specifies which arena
   to allocate from; when NULL, all default arenas are considered.

   @size        Size of requested block
   @arena_name  Name of arena to allocate in (can be NULL)
   Returns address of allocated block, NULL on error. */
void *kmalloc(size_t size, char const *arena_name);

/* krealloc(): Reallocate memory
   This function acts like realloc(). It only tries to reallocate the block in
   the arena where it was previously allocated. Note that if NULL is returned,
   the user needs to have a copy of the original address or the memory will
   become unreachable.

   @ptr   Existing allocated block
   @size  New requested size for the block
   Returns address of reallocated block, NULL on error. */
void *krealloc(void *ptr, size_t size);

/* kfree(): Free memory allocated with kalloc() */
void kfree(void *ptr);

//---
// Extension API for new areas and statistics
//---

typedef struct {
	/* Functions managing the arena. The last argument is the [data]
	   attribute in this structure. */

	/* kmalloc() handles size == 0 */
	void * (*malloc)(size_t size, void *data);
	/* krealloc() handles ptr == NULL, as well as newsize == 0 */
	void * (*realloc)(void *ptr, size_t newsize, void *data);
	/* kfree() handles ptr == NULL*/
	void (*free)(void *ptr, void *data);

	/* Name, should be unique; gint reserves names starting with "_" */
	char const *name;
	/* Start and end of arena. This is used to find the proper arena to
	   free from in kfree(). This cannot be NULL except for the OS heap as
	   the exact addresses are unknown. */
	void *start, *end;
	/* Pointer to arena-provided data, passed to malloc() and free() */
	void *data;
	/* Whether to consider this arena when performing default allocations
	   (kmalloc() with arena_name == NULL) */
	int is_default;

	/* Statistics maintained by kmalloc() */
	struct kmalloc_stats {
		int live_blocks;
		int peak_live_blocks;
		int total_volume;
		int total_blocks;
		int total_failures;
	} stats;

} kmalloc_arena_t;

//---
// Internal API
//---

/* kmalloc_init(): Initialize the dynamic allocator */
void kmalloc_init(void);

/* kmalloc_add_arena(): Add a new arena to the heap source
   Adds a fully-initialized arena to the heap source. The priority of the new
   arena compared to other default arenas is not specified. Returns true on
   success, false if the maximum number of arenas has been reached. */
bool kmalloc_add_arena(kmalloc_arena_t *arena);

#endif /* GINT_KMALLOC */
