//---
//	gint:intc - Interrupt Controller
//---

#ifndef GINT_INTC
#define GINT_INTC

//---
//	Interrupt names
//---

/* Because of SH3/SH4 differences and the different ways interrupts are mapped
   to bits of IPR and IMR registers, gint defines the following names for
   interrupt signals, and their corresponding IPR and IMR bits are defined in
   the INTC driver. */
enum {
	/* Timer Unit [TMU] */
	INTC_TMU_TUNI0,
	INTC_TMU_TUNI1,
	INTC_TMU_TUNI2,
	/* Extra Timer Unit [TMU]; SH7305-only except for ETMU_TUNI0 */
	INTC_ETMU_TUNI0,
	INTC_ETMU_TUNI1,
	INTC_ETMU_TUNI2,
	INTC_ETMU_TUNI3,
	INTC_ETMU_TUNI4,
	INTC_ETMU_TUNI5,
	/* DMA Controller [DMA0]; a single IPR is used for DEI0..3, and another
	   IPR is used for DEI4..5 and DADERR together */
	INTC_DMA_DEI0,
	INTC_DMA_DEI1,
	INTC_DMA_DEI2,
	INTC_DMA_DEI3,
	INTC_DMA_DEI4,
	INTC_DMA_DEI5,
	INTC_DMA_DADERR,
	/* Real-Time Clock [RTC]; a single IPR covers all 3 interrupts */
	INTC_RTC_ATI,
	INTC_RTC_PRI,
	INTC_RTC_CUI,
};

//---
//	Interrupt control functions
//---

/* intc_priority(): Configure the level of interrupts

   This function changes the interrupt level of the requested interrupt. Make
   sure you are aware of interrupt assignments to avoid breaking other code.
   This function is mainly used by drivers to enable the interrupts that they
   support.

   The interrupt level should be in the range 0 (disabled) .. 15 (highest
   priority). On SH7305, if the level is not 0, this function also clears the
   associated interrupt mask.

   @intname  Name of the targeted interrupt, from the enumeration above
   @level    Requested interrupt level
   Returns the interrupt level that was assigned before the call. */
int intc_priority(int intname, int level);

#endif /* GINT_INTC */
