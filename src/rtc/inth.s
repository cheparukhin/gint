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
	mov.l	.gint_inth_callback, r0
	mov.l	@r0, r0
	mov.l	1f, r4
	mov.l	2f, r5
	jsr	@r0
	nop

	/* Jump to another gate to finish the work:
	   - 0xc is the size of storage below
	   - 0x20 is the size of the gap before next gate (alarm interrupt) */
	mov	#0x2c, r2
	braf	r2
	nop

1:	.long 0			/* Callback function: edited dynamically */
2:	.long 0			/* Argument to callback function */

.gint_inth_callback:
	.long _gint_inth_callback

_inth_rtc_pri_helper:

	/* Save the return value */
	mov	r0, r3

.clear:
	/* Clear the interrupt flag */
	mov.l	.RCR2, r1
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
	rts
	nop

.RCR2:	.long 0xa413fede	/* RCR2 address, edited at startup on SH3 */
