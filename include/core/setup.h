//---
//	gint:core:setup - Installing and unloading the library
//---

#ifndef GINT_CORE_SETUP
#define GINT_CORE_SETUP

#include <defs/types.h>

/* Prototypes for the library management functions are in <gint/gint.h> */

/* gint_setvbr()
   Changes the VBR address and the calls the configuration function while
   interrupts are disabled. The configuration function must change either the
   priority registers or the interrupt masks, and make sure that all the
   interrupts that it leaves enabled are handled by the new VBR handlers.

   @vbr		New VBR address
   @configure	Configuration function
   Returns the previous VBR address. */
uint32_t gint_setvbr(uint32_t vbr, void (*configure)(void));

#endif /* GINT_CORE_SETUP */
