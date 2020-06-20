//---
//	gint:drivers - General tools for drivers
//---

#ifndef GINT_DRIVERS
#define GINT_DRIVERS

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

/* Driver procedure flow

   Drivers are initialized in priority order, and in linking order within the
   the same priority level (which is pretty much undefined). Every driver's
   priority level must be higher than those of its dependencies; the numbers
   are fixed, see the documentation for level assignments.

   At initialization, drivers are first called to wait for the hardware to
   become available before initialization. Then the system state is saved. We
   still support SH3-based SH7705-like MPUs, so a function init_sh3() is called
   for every driver that need to make adjustments to support them. Finally, the
   driver is initialized. The calls are as follow; every function pointer can
   be NULL in which case it is ignored.

      1. wait()
      2. ctx_save(sys_ctx)
      3. driver_sh3() [SH3-based fx9860g]
      4. init()

   During the execution, gint_switch() can be called to temporarily give back
   control to the OS. In this case, the state of each driver is saved to a
   context from gint, then restored from there afterwards.

       5. wait()
       6. ctx_save(gint_ctx)
       7. ctx_restore(sys_ctx)
      (stuff happening outside of gint)
       8. wait()
       9. ctx_save(sys_ctx)
      10. ctx_restore(gint_ctx)

   When finally the driver is unloaded, the system context is restored.

      11. wait()
      12. ctx_restore(sys_ctx)

   The wait() function is called both when gint has control and when the OS has
   control; thus, it must not rely on any internal state other than the
   hardware itself.

   The ctx_save() and ctx_restore() function are called with interrupts
   disabled (IMASK=15) so you should not rely on interrupts. However, TLB
   misses are still enabled so you can rely on TLB updates. */

/* gint_driver_t: Metadata and interface of kernel drivers */
typedef struct
{
	/* Driver name */
	char const *name;

	/* SH3-specific initialization step. May be NULL. */
	void (*driver_sh3)(void);
	/* Should initialize the hardware so that the driver can start working.
	   Usually installs interrupt handlers and configures interrupts. Only
	   called once when the add-in starts. May be NULL. */
	void (*init)(void);

	/* Should wait for the hardware to become available. Called both under
	   gint control and OS control every time control is passed around. It
	   is used for instance to wait for DMA transfers. May be NULL. */
	void (*wait)(void);

	/* System's context and gint's context. These should point to enough
	   memory to store a full driver state each. Used when switching from
	   the system to gint and back to the main menu. If they don't need to
	   be initialized, put them in gint's uninitialized BSS section using
	   the GBSS macro of <gint/defs/attributes.h>. May be NULL only if both
	   ctx_save() and ctx_restore() are NULL. */
	void *sys_ctx;
	void *gint_ctx;

	/* Must save the state of as much driver-controlled hardware as
	   possible (memory-mapped MPU registers, port state, etc). This
	   function is called to save the system's hardware state and gint's
	   hardware state when moving from one into the other. The parameter
	   is always either sys_ctx or gint_ctx. */
	void (*ctx_save)(void *ctx);
	/* Must restore the state of the driver as saved by ctx_save(). */
	void (*ctx_restore)(void *ctx);

} GPACKED(4) gint_driver_t;

/* GINT_DECLARE_DRIVER(): Declare a driver to the kernel

   Use this macro to declare a driver by passing it the name of a gint_driver_t
   structure. This macro moves the structure to the .gint.drivers.* sections,
   which are automatically traversed at startup.

   The level argument represents the priority level: lower numbers mean that
   drivers will be loaded sooner. This numbering allows a primitive form of
   dependency for drivers. You need to specify a level which is strictly
   higher than the level of all the drivers you depend on. */
#define GINT_DECLARE_DRIVER(level, name) \
	GSECTION(".gint.drivers." #level) extern gint_driver_t name;

/* GINT_DRIVER_SH3(): Declare an init_sh3() function
   This macro is NULL on fxcg50, so that the named function can be defined
   under #ifdef FX9860G while keeping the structure clean. */

#ifdef FXCG50
#define GINT_DRIVER_SH3(name) NULL
#else
#define GINT_DRIVER_SH3(name) name
#endif

#endif /* GINT_DRIVERS */
