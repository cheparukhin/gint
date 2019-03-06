//---
//	core:bootlog - Boot-time on-screen log for extreme debugging
//---

#ifndef GINT_CORE_BOOTLOG
#define GINT_CORE_BOOTLOG

#include <gint/defs/attributes.h>

/* bootlog_loaded() - section loading stage
   Called when RAM sections have been wiped and copied */
void bootlog_loaded(void);

/* bootlog_unmapped() - ROM mapping stage failed
   Called if not enough ROM pages have been mapped.
   @rom  Amount of mapped ROM, in bytes
   @size Minimum mapping required */
void bootlog_unmapped(int rom, int size);

/* bootlog_mapped() - ROM mapping stage
   Called after all ROM pages have been traversed. All of them may not have
   been mapped.
   @rom  Amount of mapped ROM, in bytes
   @ram  Amount of mapped RAM, in bytes */
void bootlog_mapped(uint32_t rom, uint32_t ram);

/* bootlog_kernel() - gint loading stage
   Called when gint has been installed, the VBR switched and the interrupt
   handlers set up. */
void bootlog_kernel(void);

/* bootlog_driver() - driver load
   Called for very loaded driver. */
void bootlog_driver(const char *driver_name, const char *status);
void bootlog_driver_summary(void);

/* All these functions are enabled only if GINT_BOOT_LOG is defined */
#ifndef GINT_BOOT_LOG
  #define bootlog_loaded(...)
  #define bootlog_unmapped(...)
  #define bootlog_mapped(...)
  #define bootlog_kernel(...)
  #define bootlog_driver(...)
  #define bootlog_driver_summary(...)
#endif

#endif /* GINT_CORE_BOOTLOG */
