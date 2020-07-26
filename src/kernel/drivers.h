//---
//	core:drivers - Driver utilities for the kernel
//
//	These are internal definitions; for general driver definitions, see
//	<gint/drivers.h> instead.
//---

#ifndef GINT_CORE_DRIVERS
#define GINT_CORE_DRIVERS

#include <gint/drivers.h>

/* Linker script symbols for drivers by increasing levels of priority */
extern gint_driver_t bdrv, edrv;

/* Iterate on drivers in increasing level of priority */
#define driver_asc(var) \
	(gint_driver_t *var = &bdrv; var < &edrv; var++)
/* Iterate on drivers in decreasing level of priority */
#define driver_dsc(var) \
	(gint_driver_t *var = &edrv; (--var) >= &bdrv;)

#endif /* GINT_CORE_DRIVERS */
