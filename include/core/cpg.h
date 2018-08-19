//---
//	gint:core:cpg - Clock Pulse Generator
//---

#ifndef GINT_CORE_CPG
#define GINT_CORE_CPG

#include <defs/types.h>

//---
//	SH7705 Clock Pulse Generator. Refer to:
//	  "Renesas SH7705 Group Hardware Manual"
//	  Section 9: "Interrupt Controller (INTC)"
//---

/* sh7705_cpg_t - Clock Pulse Generator registers */
typedef volatile struct
{
	word_union(FRQCR,
		uint16_t	:3;
		uint16_t CKOEN	:1;	/* Clock Output Enable */
		uint16_t	:2;
		uint16_t STC	:2;	/* PLL multiplication ratio */
		uint16_t	:2;
		uint16_t IFC	:2;	/* Internal clock divider */
		uint16_t	:2;
		uint16_t PFC	:2;	/* Peripheral clock divider */
	);

} PACKED(4) sh7705_cpg_t;

#define SH7705_CPG (*((sh7705_cpg_t *)0xffffff80))

//---
//	SH7305 Clock Pulse Generator. Refer to:
//	  "Renesas SH7724 User's Manual: Hardware"
//	  Section 17: "Clock Pulse Generator (CPG)"
//---

/* sh7305_cpg_t - Clock Pulse Generator registers
   Fields marked with [*] don't have the meaning described in the SH7724
   documentation. */
typedef volatile struct
{
	lword_union(FRQCRA,
		uint32_t KICK	:1;	/* Flush FRQCRA modifications */
		uint32_t	:1;
		uint32_t STC	:6;	/* PLL multiplication [*] */
		uint32_t IFC	:4;	/* Iphi divider 1 [*] */
		uint32_t	:4;
		uint32_t SFC	:4;	/* Sphi divider 1 [*] */
		uint32_t BFC	:4;	/* Bphi divider 1 [*] */
		uint32_t	:4;
		uint32_t P1FC	:4;	/* Pphi divider 1 [*] */
	);
	pad(0x20);

	lword_union(PLLCR,
		uint32_t	:17;
		uint32_t PLLE	:1;	/* PLL Enable */
		uint32_t	:1;
		uint32_t FLLE	:1;	/* FLL Enable */
		uint32_t	:10;
		uint32_t CKOFF	:1;	/* CKO Output Stop */
		uint32_t	:1;
	);
	pad(0x28);

	lword_union(FLLFRQ,
		uint32_t	:16;
		uint32_t SELXM	:2;	/* FLL output division */
		uint32_t	:3;
		uint32_t FLF	:11;	/* FLL Multiplication Ratio */
	);

} PACKED(4) sh7305_cpg_t;

#define SH7305_CPG (*((sh7305_cpg_t *)0xa4150000))

#endif /* GINT_CORE_CPG */
