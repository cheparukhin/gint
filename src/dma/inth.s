/*
**  gint:dma:inth - Interrupt handler for the DMA
**  An easy one, just clears some flags and marks all transfers as finished.
*/

.global _inth_dma_te
.global _inth_dma_ae
.section .gint.blocks, "ax"
.align 4

/* DMA TRANSFER ENDED INTERRUPT HANDLER - 32 BYTES */

_inth_dma_te:
	/* Clear the TE flag and DMA Enable in CHCR */
	mov.l	1f, r1
	mov.l	@r1, r0
	mov	#-4, r2
	and	r2, r0
	mov.l	r0, @r1

	/* Clear the AE and NMIF flags in OR */
	mov.l	2f, r1
	mov.w	@r1, r0
	mov	#-7, r2
	and	r2, r0
	mov.w	r0, @r1

	rte
	nop

1:	.long 0			/* CHCR, set dynamically */
2:	.long 0xfe008060	/* DMA.OR */

/* DMA ADDRESS ERROR INTERRUPT HANDLER - 18 BYTES */

_inth_dma_ae:
	/* Call a fixed function */
	sts.l	pr, @-r15
	mov.l	1f, r1
	jsr	@r1
	nop
	lds.l	@r15+, pr

	rte
	nop

	.zero 14

1:	.long _gint_dma_ae
