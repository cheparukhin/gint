//---
//	gint:drivers:iokbd - I/O ports-driven keyboard scanner
//
//	This is for SH3 only. It reads key pressed from ports A/B/M.
//---

#ifndef GINT_DRIVERS_IOKBD
#define GINT_DRIVERS_IOKBD

#include <gint/defs/types.h>

/* iokbd_scan() - scan ports A/B/M to generate 12 rows of key data
   Numbering of rows is consistent with the keyboard.
   @scan  12-byte buffer filled with row data */
void iokbd_scan(uint8_t *scan);

#endif /* GINT_DRIVERS_IOKBD */
