//---
//	gint:drivers - General tools for drivers
//---

#ifndef GINT_DRIVERS
#define GINT_DRIVERS

#include <defs/attributes.h>
#include <defs/types.h>

/* gint_driver_t - driver meta-information used by gint */
typedef struct
{
	/* Driver name */
	const char *name;

	/* Size of a context object for the driver */
	uint ctx_size;

	/* System context. The driver has to allocate a buffer of size at least
	   ctx_size, where gint stores the system's configuration. It is
	   advised to place this buffer in the .gint_bss section using the GBSS
	   macro of <defs/attributes.h> if it doesn't need to be initialized */
	void *sys_ctx;

	/* ctx_save() - save the driver's hardware support
	   This function is provided by the driver to save the state of its
	   hardware support (memory-mapped MPU registers, port state, etc).
	   @ctx  A buffer of size ctx_size */
	void (*ctx_save)(void *ctx);

	/* ctx_restore() - restore a saved context
	   This function is provided by the driver to restore the state saved
	   by ctx_save(). It can alter the contents of the buffer freely.
	   @ctx  A context buffer filled by ctx_save() */
	void (*ctx_restore)(void *ctx);

} PACKED(4) gint_driver_t;

/* GINT_DECLARE_DRIVER() - make a driver visible to gint
   Use this macro to expose a driver by passing it the name of a gint_driver_t
   structure. This macro moves the structure to the .gint.drivers section,
   which is automatically traversed at startup */
#define GINT_DECLARE_DRIVER(name) \
	SECTION(".gint.drivers") extern gint_driver_t name;

#endif /* GINT_DRIVERS */
