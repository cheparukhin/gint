//---
//	gint:core:pfc - Pin Function Controller
//
//	The Pin Function Controller has a simple register interface, the main
//	difficulty is still understanding the role of its pins.
//---

#ifndef GINT_CORE_PFC
#define GINT_CORE_PFC

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
//	SH7705 Pin Function Controller. Refer to:
//	  "Renesas SH7705 Group Hardware Manual"
//	  Section 19: "Pin Function Controller"
//---

typedef volatile struct
{
	/* Control registers */
	uint16_t PACR;
	uint16_t PBCR;
	uint16_t PCCR;
	uint16_t PDCR;
	uint16_t PECR;
	uint16_t PFCR;
	uint16_t PGCR;
	uint16_t PHCR;
	uint16_t PJCR;
	uint16_t PKCR;
	uint16_t PLCR;
	uint16_t SCPCR;		/* Port SC control register */
	uint16_t PMCR;
	uint16_t PNCR;

	pad(4);

	/* Data registers */
	uint8_t PADR;
	pad(1);
	uint8_t PBDR;
	pad(1);
	uint8_t PCDR;
	pad(1);
	uint8_t PDDR;
	pad(1);
	uint8_t PEDR;
	pad(1);
	uint8_t PFDR;
	pad(1);
	uint8_t PGDR;
	pad(1);
	uint8_t PHDR;
	pad(1);
	uint8_t PJDR;
	pad(1);
	uint8_t PKDR;
	pad(1);
	uint8_t PLDR;
	pad(1);
	uint8_t SCPDR;		/* Port SC data register */
	pad(1);
	uint8_t PMDR;
	pad(1);
	uint8_t PNDR;
	pad(1);

} GPACKED(4) sh7705_pfc_t;

#define SH7705_PFC (*((sh7705_pfc_t *)0xa4000100))

//---
//	TODO: Document the SH7305 Pin Function Controller
//---

#endif /* GINT_CORE_PFC */
