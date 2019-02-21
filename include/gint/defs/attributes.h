//---
//	gint:defs:attributes - Macros for compiler-specific attributes
//---

#ifndef GINT_DEFS_ATTRIBUTES
#define GINT_DEFS_ATTRIBUTES

/* Objects from specific sections */
#define GSECTION(x)	__attribute__((section(x)))
/* Objects from the .gint.data and .gint.bss sections */
#define GDATA		__attribute__((section(".gint.data")))
#define GBSS		__attribute__((section(".gint.bss")))
/* Additional sections that are only needed on SH3 */
#define GDATA3		__attribute__((section(".gint.data.sh3")))
#define GBSS3		__attribute__((section(".gint.bss.sh3")))

/* Unused parameters or variables */
#define GUNUSED		__attribute__((unused))
/* Functions that *must* be inlined */
#define GINLINE		__attribute__((always_inline)) inline

/* Aligned variables */
#define GALIGNED(x)	__attribute__((aligned(x)))

/* Packed structures. I require explicit alignment because if it's unspecified,
   GCC cannot optimize access size, and reads to memory-mapped I/O with invalid
   access sizes silently fail - honestly you don't want this to happen */
#define GPACKED(x)	__attribute__((packed, aligned(x)))

#endif /* GINT_DEFS_ATTRIBUTES */
