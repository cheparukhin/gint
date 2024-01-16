//---
//	core:kernel - Kernel functions
//---

#ifndef GINT_CORE_KERNEL
#define GINT_CORE_KERNEL

/* gint_load_onchip_sections(): Initialize on-chip memory sections */
void gint_load_onchip_sections(void);

/* kinit(): Install and start gint */
void kinit(void);

/* kquit(): Quit gint and give back control to the system */
void kquit(void);

#endif /* GINT_CORE_KERNEL */
