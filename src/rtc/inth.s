/*
**  gint:rtc:inth - Interrupt handler for the Real-Time Clock
**  This one is fairly simple, just a flag to clear and potentially a timer to
**  stop if the callback returns non-zero.
*/

	.global _inth_rtc_pri
	.global _inth_rtc_pri_helper
	.section .gint.blocks, "ax"
	.align 4

/* RTC PERIODIC INTERRUPT HANDLER - 56 BYTES */

_inth_rtc_pri:
	/* Invoke the callback function with its argument */
	sts.l	pr, @-r15
	mov.l	1f, r0
	mov.l	2f, r4
	jsr	@r0
	nop

	/* Save the return value */
	mov	r0, r3

	/* Prepare to clear the interrupt flag */
	mov.l	3f, r1

	/* Jump to another gate to finish the work:
	   - 0xc is the size of storage below
	   - 0x20 is the size of the next gate (alarm interrupt) */
	mov	#0x2c, r2
	braf	r2
	nop

1:	.long 0			/* Callback function: edited dynamically */
2:	.long 0			/* Argument to callback function */
3:	.long 0xa413fede	/* RCR2 address, edited at startup on SH3 */

_inth_rtc_pri_helper:

.clear:
	/* Clear the interrupt flag */
	mov.b	@r1, r0
	tst	#0x80, r0
	and	#0x7f, r0
	bf.s	.clear
	mov.b	r0, @r1

	/* Stop the timer if the return value of the callback was non-zero */
	tst	r3, r3
	bt	.end
	and	#0x8f, r0
	mov.b	r0, @r1

.end:
	lds.l	@r15+, pr
	rte
	nop

	.zero 8
