//---
//	gint:core:intc - Interrupt Controller
//
//	The interrupt controller is unwieldy because SH7705 and SH7305 have a
//	completely different interface. Everything here is split up and you'll
//	have to explicitly handle both to be able to use the device.
//
//	gint's API provides higher-level and platform-agnostic interrupt
//	management. This is probably what you are looking for.
//---

#ifndef GINT_CORE_INTC
#define GINT_CORE_INTC

#include <defs/types.h>

//---
//	SH7705 Interrupt Controller. Refer to:
//	  "Renesas SH7705 Group Hardware Manual"
//	  Section 6: "Interrupt Controller (INTC)"
//---

/* sh7705_intc_ipc_t - Interrupt Priority Controller
   A set of 16-bit register that control the interrupt priorities. The SH7705's
   IPC has its registers scattered everywhere in the memory, so there is a
   pointer for each register. The SH7305 needs only one pointer for the whole
   IPC because the registers are in a contiguous area. */
typedef struct
{
	volatile word_union(*IPRA,
		uint TMU0	:4;	/* Timer 0 */
		uint TMU1	:4;	/* Timer 1 */
		uint TMU2	:4;	/* Timer 2 */
		uint RTC	:4;	/* Real-Time Clock */
	);

	volatile word_union(*IPRB,
		uint WDT	:4;	/* Watchdog Timer */
		uint REF	:4;	/* BSC Refresh Request, SDRAM (?) */
		uint		:4;
		uint		:4;
	);

	volatile word_union(*IPRC,
		uint IRQ3	:4;	/* Interrupt request 3 */
		uint IRQ2	:4;	/* Interrupt request 2 */
		uint IRQ1	:4;	/* Interrupt request 1 */
		uint IRQ0	:4;	/* Interrupt request 0 */
	);

	volatile word_union(*IPRD,
		uint PINT0_7	:4;	/* External interrupt pins 0 to 7 */
		uint PINT8_15	:4;	/* External interrupt pins 8 to 15 */
		uint IRQ5	:4;	/* Interrupt request 5 */
		uint IRQ4	:4;	/* Interrupt request 4 */
	);

	volatile word_union(*IPRE,
		uint DMAC	:4;	/* Direct Memory Access Controller */
		uint SCIF0	:4;	/* Serial Communication Interface 0 */
		uint SCIF2	:4;	/* Serial Communication Interface 2 */
		uint ADC	:4;	/* Analog/Decimal Converter */
	);

	volatile word_union(*IPRF,
		uint		:4;
		uint		:4;
		uint USB	:4;	/* USB Controller */
		uint		:4;
	);

	volatile word_union(*IPRG,
		uint TPU0	:4;	/* Timer Pulse Unit 0 */
		uint TPU1	:4;	/* Timer Pulse Unit 1 */
		uint		:4;
		uint		:4;
	);

	volatile word_union(*IPRH,
		uint TPU2	:4;	/* Timer Pulse Unit 2 */
		uint TPU3	:4;	/* Timer Pulse Unit 3 */
		uint		:4;
		uint		:4;
	);

} PACKED(4) sh7705_intc_ipc_t;

/* sh7705_intc_icr1_t - Interrupt Control Register 1 (general) */
typedef word_union(sh7705_intc_icr1_t,
	uint MAI	:1;		/* Mask All Interrupts */
	uint IRQLVL	:1;		/* Interrupt Request Level Detect */
	uint BLMSK	:1;		/* Enable NMI when BL is set */
	uint		:1;
	uint IRQ5E	:2;		/* IRQ 5 Edge Detection */
	uint IRQ4E	:2;		/* etc. */
	uint IRQ3E	:2;
	uint IRQ2E	:2;
	uint IRQ1E	:2;
	uint IRQ0E	:2;
);

/* sh7705_intc_t - the SH7705 interrupt controller */
typedef struct
{
	/* All interrupt priority registers */
	union {
		sh7705_intc_ipc_t	_;
		volatile uint16_t	*IPRS[8];
	} PACKED(4);

	/* Control registers */
	volatile sh7705_intc_icr1_t	*ICR1;

} PACKED(4) sh7705_intc_t;



//---
//	SH7305 Interrupt Controller. Refer to:
//	  "Renesas SH7724 User's Manual: Hardware"
//	  Section 13: "Interrupt Controller (INTC)"
//---

/* sh7305_intc_ipc_t - Interrupt Priority Controller
   Some of the fields have been left unnamed because they correspond to SH7724
   peripheral modules that are *very* unlikely to even exist in the SH7305, let
   alone by of any use to us */
typedef struct
{
	word_union(IPRA,
		uint TMU0_0	:4;	/* TMU0 Channel 0 */
		uint TMU0_1	:4;	/* TMU0 Channel 1 */
		uint TMU0_2	:4;	/* TMU0 Channel 2 */
		uint IrDA	:4;	/* Infrared Communication */
	);
	pad(2);

	word_union(IPRB,
		uint		:4;	/* JPEG Processing Unit */
		uint LCDC	:4;	/* LCD Controller */
		uint DMAC1A	:4;	/* Direct Memory Access Controller 1 */
		uint		:4;	/* Blending Engine Unit */
	);
	pad(2);

	word_union(IPRC,
		uint TMU1_0	:4;	/* TMU1 Channel 0 */
		uint TMU1_1	:4;	/* TMU1 Channel 1 */
		uint TMU1_2	:4;	/* TMU1 Channel 2 */
		uint		:4;	/* Sound Processing Unit */
	);
	pad(2);

	word_union(IPRD,
		uint		:4;
		uint MMCIF	:4;	/* MultiMedia Card Interface */
		uint		:4;
		uint		:4;	/* ATAPI Interface */
	);
	pad(2);

	word_union(IPRE,
		uint DMAC0A	:4;	/* Direct Memory Access Controller 0 */
		uint		:4;	/* Various graphical engines */
		uint SCIFA3	:4;	/* SCIFA channel 3 interrupt */
		uint		:4;	/* Video Processing Unit */
	);
	pad(2);

	word_union(IPRF,
		uint KEYSC	:4;	/* Key Scan Interface */
		uint DMACOB	:4;	/* DMAC0 transfer/error info */
		uint USB0_1	:4;	/* USB controller */
		uint CMT	:4;	/* Compare Match Timer */
	);
	pad(2);

	word_union(IPRG,
		uint SCIF0	:4;	/* SCIF0 transfer/error info */
		uint SCIF1	:4;	/* SCIF1 transfer/error info */
		uint SCIF2	:4;	/* SCIF2 transfer/error info */
		uint		:4;	/* Video Engine Unit */
	);
	pad(2);

	word_union(IPRH,
		uint MSIOF0	:4;	/* Clock-synchronized SCIF channel 0 */
		uint MSIOF1	:4;	/* Clock-synchronized SCIF channel 1 */
		uint		:4;	/* I2C Interface channel 0 */
		uint		:4;	/* I2C Interface channel 1 */
	);
	pad(2);

	word_union(IPRI,
		uint SCIFA4	:4;	/* SCIFA channel 4 interrupt */
		uint		:4;	/* MediaRAM InterConnected Buffers */
		uint		:4;	/* Transport Stream Interface */
		uint		:4;	/* 2D Graphics Accelerator & ICB */
	);
	pad(2);

	word_union(IPRJ,
		uint		:4;	/* Capture Engine Unit */
		uint		:4;	/* Ethernet Memory Access Controller */
		uint FSI	:4;	/* FIFO-Buffered Serial Interface */
		uint SDHI1	:4;	/* SD Card Host Interface channel 1 */
	);
	pad(2);

	word_union(IPRK,
		uint RTC	:4;	/* Real-Time Clock */
		uint DMAC1B	:4;	/* DMAC1 transfer/error info */
		uint		:4;	/* MediaRAM InterConnected Buffers */
		uint SDHI0	:4;	/* SD Card Host Interface channel 0 */
	);
	pad(2);

	word_union(IPRL,
		uint SCIFA5	:4;	/* SCIFA channel 5 interrupt */
		uint		:4;
		uint TPU	:4;	/* Timer-Pulse Unit */
		uint		:4;	/* Image Extraction DMAC */
	);
	pad(2);

} PACKED(4) sh7305_intc_ipc_t;

/* sh7305_intc_masks_t - Interrupt mask management
   Writing 1 to IMR masks interrupts; writing 1 to IMCRs clears the masks.
   Writing 0 is ignored; reading from IMCRs yields undefined values */
typedef struct
{
	byte_union(IMR0,
		uint		:1;
		uint TUNI1_2	:1;	/* TMU1 overflow interrupts */
		uint TUNI1_1	:1;
		uint TUNI1_0	:1;
		uint SDHII3	:1;	/* SD Card Host 1 interrupts */
		uint SDHII2	:1;
		uint SDHII1	:1;
		uint SDHII0	:1;
	);
	pad(3);

	byte_union(IMR1,
		uint		:4;
		uint DEI3	:1;	/* DMAC0A interrupts */
		uint DEI2	:1;
		uint DEI1	:1;
		uint DEI0	:1;
	);
	pad(3);

	byte_union(IMR2,
		uint		:7;
		uint SCIFA0	:1;	/* Asynchronous Serial interrupts */
	);
	pad(3);

	byte_union(IMR3,
		uint DEI3	:1;	/* DMAC1A interrupts */
		uint DEI2	:1;
		uint DEI1	:1;
		uint DEI0	:1;
		uint		:4;
	);
	pad(3);

	byte_union(IMR4,
		uint		:1;
		uint TUNI0_2	:1;	/* TMU0 overflow interrupts */
		uint TUNI0_1	:1;
		uint TUNI0_0	:1;
		uint		:3;
		uint LCDCI	:1;	/* LCD Controller Interrupt */
	);
	pad(3);

	byte_union(IMR5,
		uint KEYI	:1;	/* Key Interface */
		uint DADERR	:1;	/* DMAC0B interrupts */
		uint DEI5	:1;
		uint DEI4	:1;
		uint		:1;
		uint SCIF2	:1;	/* Serial Communication Interface */
		uint SCIF1	:1;
		uint SCIF0	:1;
	);
	pad(3);

	byte_union(IMR6,
		uint		:2;
		uint		:1;
		uint SCIFA4	:1;	/* SCIFA4 interrupt */
		uint		:1;
		uint		:1;
		uint MSIOFI0	:1;	/* Clock-synchronized SCIF channel 0 */
		uint MSIOFI1	:1;	/* Clock-synchronized SCIF channel 1 */
	);
	pad(3);

	uint8_t IMR7;
	pad(3);

	byte_union(IMR8,
		uint SDHII3	:1;	/* SD Card Host 0 interrupts */
		uint SDHII2	:1;
		uint SDHII1	:1;
		uint SDHII0	:1;
		uint 		:2;
		uint SCFIA5	:1;	/* SCIFA5 interrupt */
		uint FSI	:1;	/* FIFO-Buffered Serial Interface */
	);
	pad(3);

	byte_union(IMR9,
		uint 		:3;
		uint CMTI	:1;	/* Compare Match Timer Interrupt */
		uint 		:1;
		uint USI1	:1;	/* USB1 */
		uint USI0	:1;	/* USB0 */
		uint 		:1;
	);
	pad(3);

	byte_union(IMR10,
		uint 		:1;
		uint DADERR	:1;	/* DMAC1B interrupts */
		uint DEI5	:1;
		uint DEI4	:1;
		uint		:1;
		uint ATI	:1;	/* RTC Alarm interrupt */
		uint PRI	:1;	/* RTC Periodic interrupt */
		uint CUI	:1;	/* RTC Carry interrupt */
	);
	pad(3);

	byte_union(IMR11,
		uint 		:5;
		uint TPUI	:1;	/* Timer-Pulse Unit */
		uint		:2;
	);
	pad(3);

	uint8_t IMR12;

} PACKED(4) sh7305_intc_masks_t;

/* sh7305_intc_userimask_t - User Interrupt Mask
   Sets the minimum required level for interrupts to be accepted.

   WARNING: Writing to this register is only allowed when the upper bits of the
   operand (ie. the new value of USERIMASK) are 0xa5; otherwise, the write is
   ignored. To modify the value of this register, do not access the bit field
   directly, backup the variable and modify it:

	void set_user_imask(int new_level)
	{
		sh7305_intc_userimask_t mask = *(INTC._7305.USERIMASK);
		mask._0xa5	= 0xa5;
		mask.UIMASK	= new_level & 0x0f;
		*(INTC._7305.USERIMASK) = mask;
	}
*/
typedef lword_union(sh7305_intc_userimask_t,
	uint _0xa5	:8;		/* Always set to 0xa5 before writing */
	uint		:16;
	uint UIMASK	:4;		/* User Interrupt Mask Level */
	uint		:4;
);

/* sh7305_intc_t - the SH7305 interrupt controller */
typedef struct
{
	/* Interrupt priority registers */
	union {
		volatile sh7305_intc_ipc_t	*_;
		volatile uint16_t		*IPRS;
	};

	/* Interrupt mask & mask clear registers */
	volatile sh7305_intc_masks_t		*MSK;
	volatile sh7305_intc_masks_t		*MSKCLR;

	/* Other registers */
	volatile sh7305_intc_userimask_t	*USERIMASK;

} PACKED(4) sh7305_intc_t;

#endif /* GINT_CORE_INTC */
