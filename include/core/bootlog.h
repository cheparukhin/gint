//---
//	gint:core:bootlog - Boot-time on-screen log for extreme debugging
//---

#ifndef GINT_CORE_BOOTLOG
#define GINT_CORE_BOOTLOG

/* bootlog_loaded() - Section loading stage
   Called when RAM sections have been wiped and copied */
void bootlog_loaded(void);

/* bootlog_mapped() - ROM mapping stage
   Called after all ROM pages have been traversed. All of them may not have
   been mapped.
   @rom  Amount of mapped ROM, in bytes
   @ram  Amount of mapped RAM, in bytes */
void bootlog_mapped(uint32_t rom, uint32_t ram);

/* bootlog_kernel() - Gint loading stage
   Called when gint has been installed, the VBR switched and the interrupt
   handlers set up. */
void bootlog_kernel(void);

#endif /* GINT_CORE_BOOTLOG */
