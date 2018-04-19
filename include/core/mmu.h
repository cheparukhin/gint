//---
//	gint:core:mmu - MMU-related definitions
//
//	gint does not touch the MMU because the risk of interfering with the
//	system is deemed too high. However, to ensure that the add-in runs
//	properly, checks using read-only access to the MMU are performed.
//---

#ifndef GINT_CORE_MMU
#define GINT_CORE_MMU

#include <defs/attributes.h>
#include <defs/types.h>

/* utlb_addr_t - address part of a UTLB entry */
typedef struct
{
	uint VPN	:22;
	uint D		:1;
	uint V		:1;
	uint ASID	:8;

} PACKED(4) utlb_addr_t;

/* utlb_addr() - get the P4 address of a UTLB address entry
   @E  Entry number (should be in range 0..63)
   Returns a pointer to the entry. */
const utlb_addr_t *utlb_addr(uint E);

/* utlb_data_t - data part of a UTLB entry */
typedef struct
{
	uint		:3;
	uint PPN	:19;
	uint		:1;
	uint V		:1;
	uint SZ1	:1;
	uint PR		:2;
	uint SZ2	:1;
	uint C		:1;
	uint D		:1;
	uint SH		:1;
	uint WT		:1;

} PACKED(4) utlb_data_t;

/* utlb_data() - get the P4 address of a UTLB data entry
   @E  Entry number (should be in range 0..63)
   Returns a pointer to the entry. */
const utlb_data_t *utlb_data(uint E);

/* utlb_mapped_memory() - count amount of mapped memory
   This function returns the amount of mapped text and data segment memory, in
   bytes. The ranges are defined as follows:
     ROM  00300000:512k
     RAM  08100000:512k
   Other mappings are ignored. Both pointers may be NULL.

   @rom  Pointer to amount of mapped ROM
   @ram  Pointer to amount of mapped RAM */
void utlb_mapped_memory(uint32_t *rom, uint32_t *ram);

#endif /* GINT_CORE_MMU */
