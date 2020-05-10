//---
//	gint:drivers - General tools for drivers
//---

#ifndef GINT_DRIVERS
#define GINT_DRIVERS

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
//	Driver procedure flow
//
//	Drivers are initialized in priority order, and in linking order within
//	the same priority level (pretty much undefined). Make sure every
//	driver's priority level is higher than those of its dependencies.
//
//	At initialization, the following functions are called:
//	1. driver_sh3()            [if not NULL, SH3 fx9860g only]
//	2. ctx_save(sys_ctx)       [if not NULL]
//	3. init()                  [if not NULL]
//
//	Then, if the on-screen boot log is enabled, the status() function is
//	called and the returned string is displayed (21 characters max).
//	4. status()                [if not NULL, if GINT_BOOT_LOG is defined]
//
//	If the gint_switch() function is called to temporarily give back
//	control to the operating system, the state of each driver is saved to
//	the stack, then restored from there.
//	5. ctx_save(stack)         [if not NULL]
//	6. ctx_restore(stack)      [if not NULL]
//
//	When finally the driver is unloaded, the system context is restored.
//	7. ctx_restore(sys_ctx)    [if not NULL]
//---

/* gint_driver_t - driver meta-information used by gint */
typedef struct
{
	/* Driver name */
	char const *name;

	/* driver_sh3() - rectify driver initialization on SH3 platforms
	   This function is called during driver initialization on SH3. It may
	   be NULL. */
	void (*driver_sh3)(void);

	/* init() - initialize the driver
	   This function is called after ctx_save() and needs not save the
	   system setting. It is typically used to initialize registers to
	   suitable values on startup. If there is no init function, this field
	   may be set to NULL */
	void (*init)(void);

	/* unload() - deinitialize the driver
	   This function is called before ctx_restore() when gint is unloaded.
	   If there is no unload function, the field may be set to NULL */
	void (*unload)(void);

	/* System's context and gint's context. These should point to enough
	   memory to store a full driver state each. These are used when
	   switching from the system to gint and back to the main menu. If they
	   don't need to be initialized, put them in gint's uninitialized BSS
	   section using the GBSS macro of <gint/defs/attributes.h>. */
	void *sys_ctx;
	void *gint_ctx;

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

	/* status() - status string generation
	   When the boot log is defined, this function is called to print
	   information returned by the driver, for debugging purposes. This is
	   expected to be a short (max 21 bytes) string because only a few
	   lines are available for all drivers.
	   Returns a pointer to a string; a static buffer is suitable. */
	char const * (*status)(void);

} GPACKED(4) gint_driver_t;

/* GINT_DECLARE_DRIVER() - make a driver visible to gint

   Use this macro to expose a driver by passing it the name of a gint_driver_t
   structure. This macro moves the structure to the .gint.drivers.* sections,
   which are automatically traversed at startup.

   The @level argument represents the priority level: lower numbers mean that
   drivers will be loaded sooner. This numbering allows a primitive form of
   dependency for drivers. You need to specify a level which is strictly
   higher than the level of all the drivers you depend on. */
#define GINT_DECLARE_DRIVER(level, name) \
	GSECTION(".gint.drivers." #level) extern gint_driver_t name;

/* GINT_DRIVER_SH3() - declare a function for SH3-rectification
   This macro makes its argument NULL on fxcg50, this way the named function
   can be defined under #ifdef FX9860G while keeping the structure clean. */

#ifdef FXCG50
#define GINT_DRIVER_SH3(name) NULL
#else
#define GINT_DRIVER_SH3(name) name
#endif

/* GINT_DRIVER_STATUS() - declare a function for status string generation
   This macro makes its argument NULL when GINT_BOOT_LOG is defuned, this way
   the named function can be defined under #ifdef GINT_BOOT_LOG while keeping
   the structure clean. */

#ifdef GINT_BOOT_LOG
#define GINT_DRIVER_STATUS(name) name
#else
#define GINT_DRIVER_STATUS(name) NULL
#endif

#endif /* GINT_DRIVERS */
