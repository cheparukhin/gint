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
//	driver's priority level is higher than those of its dependencies. In
//	the description below, every function can be NULL.
//
//	At initialization, the following functions are called:
//	1. driver_sh3() [on SH3-based fx9860g only]
//	2. ctx_save(sys_ctx)
//	3. init()
//
//	Then, if the on-screen boot log is enabled, the status() function is
//	called and the returned string is displayed (21 characters max).
//	4. status() [only if GINT_BOOT_LOG is defined]
//
//	If the gint_switch() function is called to temporarily give back
//	control to the operating system, the state of each driver is saved to
//	the stack, then restored from there.
//	5. wait()
//	6. ctx_save(gint_ctx)
//	7. ctx_restore(sys_ctx)
//	(stuff happening outside of gint)
//	8. ctx_save(sys_ctx)
//	9. ctx_restore(gint_ctx)
//
//	When finally the driver is unloaded, the system context is restored.
//	10. wait()
//	11. ctx_restore(sys_ctx)
//---

/* gint_driver_t - driver meta-information used by gint */
typedef struct
{
	/* Driver name */
	char const *name;

	/* SH3-specific preinitializaton; called before init() when running on
	   SH3. May be NULL. */
	void (*driver_sh3)(void);

	/* Must initialize the hardware so that the driver can start working.
	   This is called only once when the add-in starts, and should not save
	   hardware state (ctx_save() is called before). May be NULL. */
	void (*init)(void);

	/* This function can be used to enforce a waiting period before the
	   driver is unloaded. It is called before returning to the OS in
	   gint_switch() and if the add-in exits. May be NULL. */
	void (*wait)(void);

	/* System's context and gint's context. These should point to enough
	   memory to store a full driver state each. These are used when
	   switching from the system to gint and back to the main menu. If they
	   don't need to be initialized, put them in gint's uninitialized BSS
	   section using the GBSS macro of <gint/defs/attributes.h>. May be
	   NULL only if both ctx_save() and ctx_restore() are NULL. */
	void *sys_ctx;
	void *gint_ctx;

	/* Must save the state of as much driver-controlled hardware as
	   possible (memory-mapped MPU registers, port state, etc). This
	   function is called to save the system's hardware state and gint's
	   hardware state when moving from one into the other. The parameter
	   [ctx] is always either [sys_ctx] or [gint_ctx]. */
	void (*ctx_save)(void *ctx);

	/* Must restore the state of the driver as saved by ctx_save(). */
	void (*ctx_restore)(void *ctx);

	/* This function may generate a status string to display on the boot
	   log for debugging purposes. It should be a short (max 21 bytes)
	   string because all drivers' strings must fit on a few lines. May
	   return a pointer to a static buffer. May be NULL. */
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

/* GINT_DRIVER_SH3() - declare a function for SH3 preinitialization
   This macro makes its argument NULL on fxcg50, this way the named function
   can be defined under #ifdef FX9860G while keeping the structure clean. */

#ifdef FXCG50
#define GINT_DRIVER_SH3(name) NULL
#else
#define GINT_DRIVER_SH3(name) name
#endif

/* GINT_DRIVER_STATUS() - declare a function for status string generation
   This macro makes its argument NULL when GINT_BOOT_LOG is undefined, this way
   the named function can be defined under #ifdef GINT_BOOT_LOG while keeping
   the structure clean. */

#ifdef GINT_BOOT_LOG
#define GINT_DRIVER_STATUS(name) name
#else
#define GINT_DRIVER_STATUS(name) NULL
#endif

#endif /* GINT_DRIVERS */
