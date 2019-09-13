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
	/* Manually RTE into the panic routine, preserving SPC */
	mov.l	2f, r4
	ldc	r4, r4_bank

	/* This instruction changes register bank! */
	stc	ssr, r1
	ldc	r1, sr

	mov.l	1f, r0
	jmp	@r0
	nop

	.zero 10

1:	.long _gint_panic
2:	.long 0x1020
