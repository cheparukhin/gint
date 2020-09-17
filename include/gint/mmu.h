//---
//	gint:mmu - Memory Management Unit
//---

#ifndef GINT_MMU
#define GINT_MMU

#include <gint/mpu/mmu.h>

//---
//	Unified interface
//---

/* mmu_translate(): Get the physical address for a virtual page
   Looks for a translation with the specified virtual address as start, and
   returns the corresponding physical address. Only works if the argument is
   page-aligned.

   @virtual  Virtual page address
   Returns the page's physical address, or -1 if not mapped. */
uint32_t mmu_translate(uint32_t page);

/* mmu_uram(): Get pointer to physical start of user RAM

   Returns a pointer to the physical location behind 0x08100000. The physical
   location can be used to access without the TLB, which is useful when
   interrupts are processed with SR.BL=1. However, the location is highly
   platform-dependent. */
void *mmu_uram(void);

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

/* tlb_translate(): Get the physical address for a virtual page */
uint32_t tlb_translate(uint32_t page);

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

/* utlb_translate(): Get the physical address for a virtual page */
uint32_t utlb_translate(uint32_t page);

#endif /* GINT_MMU */
