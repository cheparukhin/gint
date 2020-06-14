//---
//	gint:mmu - Memory Management Unit
//---

#ifndef GINT_MMU
#define GINT_MMU

#include <gint/mpu/mmu.h>

//---
//	SH7705 TLB
//---

/* tlb_addr() - get the P4 address of a TLB address entry
   @way  TLB way (0..3)
   @E    Entry number (0..31)
   Returns a pointer to the entry. */
tlb_addr_t const *tlb_addr(uint way, uint E);

/* tlb_data() - get the P4 address of a TLB data entry
   @way  TLB way (0..3)
   @E    Entry number (0..31)
   Returns a pointer to the entry. */
tlb_data_t const *tlb_data(uint way, uint E);

/* tlb_mapped_memory() - count amount of mapped memory
   This function returns the amount of mapped text and data segment memory, in
   bytes. The ranges are defined as follows:
     ROM  00300000:512k
     RAM  08100000:512k
   Other mappings are ignored. Both pointers may be NULL.

   @rom  Pointer to amount of mapped ROM
   @ram  Pointer to amount of mapped RAM */
void tlb_mapped_memory(uint32_t *p_rom, uint32_t *p_ram);

//---
//	SH7305 Unified TLB
//---

/* utlb_addr() - get the P4 address of a UTLB address entry
   @E  Entry number (should be in range 0..63)
   Returns a pointer to the entry. */
const utlb_addr_t *utlb_addr(uint E);

/* utlb_data() - get the P4 address of a UTLB data entry
   @E  Entry number (should be in range 0..63)
   Returns a pointer to the entry. */
utlb_data_t const *utlb_data(uint E);

/* utlb_mapped_memory() - count amount of mapped memory
   This function returns the amount of mapped text and data segment memory, in
   bytes. The ranges are defined as follows:
     ROM  00300000:4M
     RAM  08100000:512k
   Other mappings are ignored. Both pointers may be NULL.

   @rom  Pointer to amount of mapped ROM
   @ram  Pointer to amount of mapped RAM */
void utlb_mapped_memory(uint32_t *rom, uint32_t *ram);

#endif /* GINT_MMU */
