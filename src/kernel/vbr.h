//---
//	core:vbr - VBR-related functions and linker script symbols
//---

#ifndef GINT_CORE_VBR
#define GINT_CORE_VBR

#include <gint/defs/types.h>

/* VBR addresses from the linker script */
#ifdef FX9860G
extern char gint_vbr_fx9860g[];
#endif
#ifdef FXCG50
extern char gint_vbr_fxcg50[];
extern char gint_vbr_fxcg20[];
#endif

/* The kernel's interrupt and exception handlers' entry points */
void gint_exch(void);
void gint_tlbh(void);
void gint_inth_7705(void);
void gint_inth_7305(void);

/* Size of exception and TLB handlers */
extern char gint_exch_size;
extern char gint_tlbh_size;

#endif /* GINT_CORE_VBR */
