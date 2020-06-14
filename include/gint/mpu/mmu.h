//---
//	gint:mpu:mmu - Memory Management Unit
//
//	The MMU mainly exposes the contents of the TLB for us to inspect.
//	Functions to manipulate these are exposed by <gint/mmu.h>.
//---

#ifndef GINT_MPU_MMU
#define GINT_MPU_MMU

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
//	SH7705 TLB. Refer to:
//	  "Renesas SH7705 Group Hardware Manual"
//	  Section 3: "Memory Manaegement Unit (MMU)"
//---

/* tlb_addr_t - address part of a TLB entry */
typedef struct
{
	uint VPN	:22;
	uint		:1;
	uint V		:1;
	uint ASID	:8;

} GPACKED(4) tlb_addr_t;

/* tlb_data_t - data part of a TLB entry */
typedef struct
{
	uint		:3;
	uint PPN	:19;
	uint		:1;
	uint V		:1;
	uint		:1;
	uint PR		:2;
	uint SZ		:1;
	uint C		:1;
	uint D		:1;
	uint SH		:1;
	uint		:1;

} GPACKED(4) tlb_data_t;

//---
//	SH7305 TLB. Refer to:
//	  "Renesas SH7724 User's Manual: Hardware"
//	  Section 7: "Memory Management Unit (MMU)"
//---

/* utlb_addr_t - address part of a UTLB entry */
typedef struct
{
	uint VPN	:22;
	uint D		:1;
	uint V		:1;
	uint ASID	:8;

} GPACKED(4) utlb_addr_t;

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

} GPACKED(4) utlb_data_t;

#endif /* GINT_MPU_MMU */
