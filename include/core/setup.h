//---
//	core:setup - Installing and unloading the library
//---

#ifndef GINT_CORE_SETUP
#define GINT_CORE_SETUP

#include <gint/defs/types.h>

/* Prototypes for the library management functions are in <gint/gint.h> */

/* gint_setvbr()
   Changes the VBR address and calls the configuration function while
   interrupts are disabled. The configuration function must disable all
   interrupts that the new handlers cannot handle, either by clearing the
   priority registers or by setting the interrupt masks.

   @vbr		New VBR address
   @configure	Configuration function
   Returns the previous VBR address. */
uint32_t gint_setvbr(uint32_t vbr, void (*configure)(void));

/* gint_exch(): Exception handler */
void gint_exch(void);
/* gint_tlbh(): TLB miss handler */
void gint_tlbh(void);

/* gint_inth_7705(): SH7705 exception handler */
void gint_inth_7705(void);
/* gint_inth_7305(): SH7305 exception handler */
void gint_inth_7305(void);

#endif /* GINT_CORE_SETUP */
